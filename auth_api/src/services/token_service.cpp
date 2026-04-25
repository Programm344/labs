#include "TokenService.h"
#include "../database/Database.h"
#include <jwt-cpp/jwt.h>     
#include <chrono>              
#include <iostream>            
#include <cstdlib>             //для std::getenv
#include <ctime>              

TokenService::TokenService() {
    secret_key_ = std::getenv("JWT_SECRET") 
    ? std::getenv("JWT_SECRET") 
    : "default-secret-key-change-me-in-production";

     access_token_ttl_ = std::getenv("ACCESS_TOKEN_TTL") 
        ? std::stoi(std::getenv("ACCESS_TOKEN_TTL")) 
        : 60;

         refresh_token_ttl_ = std::getenv("REFRESH_TOKEN_TTL") 
        ? std::stoi(std::getenv("REFRESH_TOKEN_TTL")) 
        : 10080;

         max_active_tokens_ = std::getenv("MAX_ACTIVE_TOKENS") 
        ? std::stoi(std::getenv("MAX_ACTIVE_TOKENS")) 
        : 5;

    std::cout << "[TokenService] Initialized with:" << std::endl;
    std::cout << "  Access TTL: " << access_token_ttl_ << " minutes" << std::endl;
    std::cout << "  Refresh TTL: " << refresh_token_ttl_ << " minutes" << std::endl;
    std::cout << "  Max active tokens: " << max_active_tokens_ << std::endl;
}

std::string TokenService::hash_token(const std::string& token) {
    
    
    std::hash<std::string> hasher;
    return std::to_string(hasher(token));
}

void TokenService::store_token_hash(const std::string& token_hash, int user_id, 
                                   int ttl_minutes, bool is_refresh) {
    
    auto now = std::chrono::system_clock::now();
    
    auto expires = now + std::chrono::minutes(ttl_minutes);
    
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
    sqlite3_bind_text(stmt, 1, token_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_bind_text(stmt, 3, now_str, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, exp_str, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, is_refresh ? 1 : 0);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

bool TokenService::is_token_revoked(const std::string& token_hash) {
    
    const char* sql = "SELECT COUNT(*) FROM revoked_tokens WHERE token_hash = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token_hash.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count > 0;  
}

void TokenService::mark_token_as_revoked(const std::string& token_hash, int user_id) {
    // добавляем в таблицу отозванных
    std::string sql = "INSERT INTO revoked_tokens (token_hash, user_id) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, user_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    // удаляем из активных токенов
    sql = "DELETE FROM active_tokens WHERE token_hash = ?";
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

TokenService::TokenPair TokenService::generate_tokens(int user_id) {
    auto now = std::chrono::system_clock::now();

     auto access_token = jwt::create()
        .set_issuer("auth_api")
        .set_type("access")
        .set_subject(std::to_string(user_id))
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::minutes(access_token_ttl_))
        .sign(jwt::algorithm::hs256{secret_key_});

         auto refresh_token = jwt::create()
        .set_issuer("auth_api")
        .set_type("refresh")
        .set_subject(std::to_string(user_id))
        .set_issued_at(now)
        .set_expires_at(now + std::chrono::minutes(refresh_token_ttl_))
        .sign(jwt::algorithm::hs256{secret_key_});

         store_token_hash(hash_token(access_token), user_id, access_token_ttl_, false);
         store_token_hash(hash_token(refresh_token), user_id, refresh_token_ttl_, true);
    
         enforce_token_limit(user_id);
    
         return {access_token, refresh_token};
}

bool TokenService::validate_access_token(const std::string& token) {
    try {
        // расшифровываем JWT
        auto decoded = jwt::decode(token);
        
        // настраиваем проверку
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key_})
            .with_issuer("auth_api")
            .with_claim("type", jwt::claim(std::string("access")));
        
        // проверяем подпись и срок действия
        verifier.verify(decoded);
        
        // проверяем, не отозван ли токен
        return !is_token_revoked(hash_token(token));
    } catch (const std::exception& e) {
        // любая ошибка = токен невалидный
        return false;
    }
}

bool TokenService::validate_refresh_token(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret_key_})
            .with_issuer("auth_api")
            .with_claim("type", jwt::claim(std::string("refresh")));
        verifier.verify(decoded);
        return !is_token_revoked(hash_token(token));
    } catch (...) {
        return false;
    }
}

int TokenService::get_user_id_from_token(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        
        return std::stoi(decoded.get_subject());
    } catch (...) {
        return -1;  
    }
}

void TokenService::revoke_token(const std::string& token) {
    int user_id = get_user_id_from_token(token);
    if (user_id != -1) {
        mark_token_as_revoked(hash_token(token), user_id);
    }
}

void TokenService::revoke_all_user_tokens(int user_id) {
    /
    const char* sql = "SELECT token_hash FROM active_tokens WHERE user_id = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);
    
    // отзываем каждый токен
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        mark_token_as_revoked(hash, user_id);
    }
    sqlite3_finalize(stmt);
    
    std::cout << "[TokenService] Revoked all tokens for user " << user_id << std::endl;
}

std::vector<ActiveTokenInfo> TokenService::get_user_active_tokens(int user_id) {
    std::vector<ActiveTokenInfo> result;
    
    // выбираем только access токены 
    const char* sql = "SELECT token_hash, created_at, expires_at FROM active_tokens "
                      "WHERE user_id = ? AND is_refresh = 0";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ActiveTokenInfo info;
        std::string full_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        info.id = full_hash.substr(0, 16);  // берем только первые 16 символов как ID
        info.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        info.expires_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        info.last_used = info.created_at;  
        result.push_back(info);
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<TokenService::TokenPair> TokenService::refresh_tokens(const std::string& refresh_token) {
    
    if (!validate_refresh_token(refresh_token)) {
        int user_id = get_user_id_from_token(refresh_token);
        if (user_id != -1) {
            //  при невалидном refresh отзываем ВСЕ токены пользователя для безопасноси
           
            revoke_all_user_tokens(user_id);
            std::cout << "[TokenService] Invalid refresh token for user " 
                      << user_id << ", revoked all tokens" << std::endl;
        }
        return std::nullopt;
    }
    
    int user_id = get_user_id_from_token(refresh_token);
    // отзываем старый refresh токен 
    revoke_token(refresh_token);
    // генерируем новую пару
    return generate_tokens(user_id);
}

void TokenService::enforce_token_limit(int user_id) {
    
    const char* sql = "SELECT COUNT(*) FROM active_tokens WHERE user_id = ? AND is_refresh = 0";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    
    if (count >= max_active_tokens_) {
        // находим самый старый токен
        const char* find_oldest = "SELECT token_hash FROM active_tokens "
                                 "WHERE user_id = ? AND is_refresh = 0 "
                                 "ORDER BY created_at ASC LIMIT 1";
        sqlite3_prepare_v2(Database::get_instance().getDB(), find_oldest, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, user_id);
        
        // удаляем
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string oldest_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            mark_token_as_revoked(oldest_hash, user_id);
            std::cout << "[TokenService] Token limit exceeded for user " 
                      << user_id << ", revoked oldest token" << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}