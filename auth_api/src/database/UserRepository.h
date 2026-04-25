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
    
    static std::optional<UserRecord> findByUsername(const std::string& username);
    static std::optional<UserRecord> findById(int id);
    static bool isUsernameUnique(const std::string& username);
    static bool isEmailUnique(const std::string& email);
    static bool updatePassword(int userId, const std::string& new_password_hash);
};