#include "UserRepository.h"
#include "Database.h"
#include <iostream>
#include <cctype>

static std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::tolower(c);
    }
    return result;
}

bool UserRepository::create(const std::string& username,
                            const std::string& email,
                            const std::string& password_hash,
                            const std::string& birthday) {
    const char* sql = "INSERT INTO users (username, email, password_hash, birthday) VALUES (?, ?, ?, ?)";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, birthday.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}

std::optional<UserRecord> UserRepository::findByUsername(const std::string& username) {
    const char* sql = "SELECT id, username, email, password_hash, birthday, created_at FROM users WHERE username = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        UserRecord user;
        user.id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.birthday = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<UserRecord> UserRepository::findById(int id) {
    const char* sql = "SELECT id, username, email, password_hash, birthday, created_at FROM users WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        UserRecord user;
        user.id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.password_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user.birthday = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool UserRepository::isUsernameUnique(const std::string& username) {
    std::string lower = toLower(username);
    const char* sql = "SELECT COUNT(*) FROM users WHERE LOWER(username) = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, lower.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count == 0;
}

bool UserRepository::isEmailUnique(const std::string& email) {
    std::string lower = toLower(email);
    const char* sql = "SELECT COUNT(*) FROM users WHERE LOWER(email) = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, lower.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count == 0;
}

bool UserRepository::updatePassword(int userId, const std::string& new_password_hash) {
    const char* sql = "UPDATE users SET password_hash = ? WHERE id = ?";
    
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(Database::get_instance().getDB(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, new_password_hash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, userId);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}