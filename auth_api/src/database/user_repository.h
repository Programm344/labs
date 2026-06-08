#pragma once
#include <string>
#include <optional>

struct UserRecord {
    int id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string birthday;
    std::string created_at;
};

class UserRepository {
public:
    static bool create(const std::string& username,
                       const std::string& email,
                       const std::string& password_hash,
                       const std::string& birthday);
    
    static std::optional<UserRecord> find_by_username(const std::string& username);
    static std::optional<UserRecord> find_by_id(int id);
    static bool is_username_unique(const std::string& username);
    static bool is_email_unique(const std::string& email);
    static bool update_password(int user_id, const std::string& new_password_hash);
};