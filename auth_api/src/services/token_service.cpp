#include "TokenService.h"
#include "../database/Database.h"
#include <jwt-cpp/jwt.h>     
#include <chrono>              
#include <iostream>            
#include <cstdlib>             //для std::getenv
#include <ctime>              

TokenService::TokenService() {
    secretKey = std::getenv("JWT_SECRET") 
    ? std::getenv("JWT_SECRET") 
    : "default-secret-key-change-me-in-production";

     accessTokenTTL = std::getenv("ACCESS_TOKEN_TTL") 
        ? std::stoi(std::getenv("ACCESS_TOKEN_TTL")) 
        : 60;

         refreshTokenTTL = std::getenv("REFRESH_TOKEN_TTL") 
        ? std::stoi(std::getenv("REFRESH_TOKEN_TTL")) 
        : 10080;

         maxActiveTokens = std::getenv("MAX_ACTIVE_TOKENS") 
        ? std::stoi(std::getenv("MAX_ACTIVE_TOKENS")) 
        : 5;

    std::cout << "[TokenService] Initialized with:" << std::endl;
    std::cout << "  Access TTL: " << accessTokenTTL << " minutes" << std::endl;
    std::cout << "  Refresh TTL: " << refreshTokenTTL << " minutes" << std::endl;
    std::cout << "  Max active tokens: " << maxActiveTokens << std::endl;
}

std::string TokenService::hashToken(const std::string& token) {
    
    
    std::hash<std::string> hasher;
    return std::to_string(hasher(token));
}

void TokenService::storeTokenHash(const std::string& tokenHash, int userId, 
                                   int ttlMinutes, bool isRefresh) {
    
    auto now = std::chrono::system_clock::now();
    
    auto expires = now + std::chrono::minutes(ttlMinutes);
    
    // преобразуем время в строку для SQLite
    auto now_t = std::chrono::system_clock::to_time_t(now);
    auto exp_t = std::chrono::system_clock::to_time_t(expires);
    
    char now_str[20], exp_str[20];
    strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", localtime(&now_t));
    strftime(exp_str, sizeof(exp_str), "%Y-%m-%d %H:%M:%S", localtime(&exp_t));
    
    // SQL запрос для вставки или обновления хеша токена
    std::string sql = "INSERT OR REPLACE INTO active_tokens "
                      "(token_hash, user_id, created_at, expires_at, is_refresh) "
                      "VALUES (?, ?, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, tokenHash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, userId);
    sqlite3_bind_text(stmt, 3, now_str, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, exp_str, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, isRefresh ? 1 : 0);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

bool TokenService::isTokenRevoked(const std::string& tokenHash) {
    
    const char* sql = "SELECT COUNT(*) FROM revoked_tokens WHERE token_hash = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, tokenHash.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count > 0;  
}

void TokenService::markTokenAsRevoked(const std::string& tokenHash, int userId) {
    // добавляем в таблицу отозванных
    std::string sql = "INSERT INTO revoked_tokens (token_hash, user_id) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, tokenHash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, userId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    // удаляем из активных токенов
    sql = "DELETE FROM active_tokens WHERE token_hash = ?";
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, tokenHash.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

TokenService::TokenPair TokenService::generateTokens(int userId) {
    auto now = std::chrono::system_clock::now();

     auto accessToken = jwt::create()
        .set_issuer("auth_api")
        .set_type("access")
        .set_subject(std::to_string(userId))
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::minutes(accessTokenTTL))
        .sign(jwt::algorithm::hs256{secretKey});

         auto refreshToken = jwt::create()
        .set_issuer("auth_api")
        .set_type("refresh")
        .set_subject(std::to_string(userId))
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::minutes(refreshTokenTTL))
        .sign(jwt::algorithm::hs256{secretKey});

         storeTokenHash(hashToken(accessToken), userId, accessTokenTTL, false);
         storeTokenHash(hashToken(refreshToken), userId, refreshTokenTTL, true);
    
         enforceTokenLimit(userId);
    
         return {accessToken, refreshToken};
}

bool TokenService::validateAccessToken(const std::string& token) {
    try {
        // расшифровываем JWT
        auto decoded = jwt::decode(token);
        
        // настраиваем проверку
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secretKey})
            .with_issuer("auth_api")
            .with_claim("type", jwt::claim(std::string("access")));
        
        // проверяем подпись и срок действия
        verifier.verify(decoded);
        
        // проверяем, не отозван ли токен
        return !isTokenRevoked(hashToken(token));
    } catch (const std::exception& e) {
        // любая ошибка = токен невалидный
        return false;
    }
}

bool TokenService::validateRefreshToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secretKey})
            .with_issuer("auth_api")
            .with_claim("type", jwt::claim(std::string("refresh")));
        verifier.verify(decoded);
        return !isTokenRevoked(hashToken(token));
    } catch (...) {
        return false;
    }
}

int TokenService::getUserIdFromToken(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        
        return std::stoi(decoded.get_subject());
    } catch (...) {
        return -1;  
    }
}

void TokenService::revokeToken(const std::string& token) {
    int userId = getUserIdFromToken(token);
    if (userId != -1) {
        markTokenAsRevoked(hashToken(token), userId);
    }
}

void TokenService::revokeAllUserTokens(int userId) {
    /
    const char* sql = "SELECT token_hash FROM active_tokens WHERE user_id = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, userId);
    
    // отзываем каждый токен
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        markTokenAsRevoked(hash, userId);
    }
    sqlite3_finalize(stmt);
    
    std::cout << "[TokenService] Revoked all tokens for user " << userId << std::endl;
}

std::vector<ActiveTokenInfo> TokenService::getUserActiveTokens(int userId) {
    std::vector<ActiveTokenInfo> result;
    
    // выбираем только access токены 
    const char* sql = "SELECT token_hash, created_at, expires_at FROM active_tokens "
                      "WHERE user_id = ? AND is_refresh = 0";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, userId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ActiveTokenInfo info;
        std::string fullHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        info.id = fullHash.substr(0, 16);  // берем только первые 16 символов как ID
        info.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        info.expiresAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        info.lastUsed = info.createdAt;  
        result.push_back(info);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<TokenService::TokenPair> TokenService::refreshTokens(const std::string& refreshToken) {
    
    if (!validateRefreshToken(refreshToken)) {
        int userId = getUserIdFromToken(refreshToken);
        if (userId != -1) {
            //  при невалидном refresh отзываем ВСЕ токены пользователя для безопасноси
           
            revokeAllUserTokens(userId);
            std::cout << "[TokenService] Invalid refresh token for user " 
                      << userId << ", revoked all tokens" << std::endl;
        }
        return std::nullopt;
    }
    
    int userId = getUserIdFromToken(refreshToken);
    // отзываем старый refresh токен 
    revokeToken(refreshToken);
    // генерируем новую пару
    return generateTokens(userId);
}

void TokenService::enforceTokenLimit(int userId) {
    
    const char* sql = "SELECT COUNT(*) FROM active_tokens WHERE user_id = ? AND is_refresh = 0";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, userId);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    
    if (count >= maxActiveTokens) {
        // находим самый старый токен
        const char* findOldest = "SELECT token_hash FROM active_tokens "
                                 "WHERE user_id = ? AND is_refresh = 0 "
                                 "ORDER BY created_at ASC LIMIT 1";
        sqlite3_prepare_v2(Database::get_instance().getDB(), findOldest, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, userId);
        
        // удаляем
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string oldestHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            markTokenAsRevoked(oldestHash, userId);
            std::cout << "[TokenService] Token limit exceeded for user " 
                      << userId << ", revoked oldest token" << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}