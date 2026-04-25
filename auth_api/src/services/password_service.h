#pragma once
#include <string>
#include <bcrypt/BCrypt.hpp>

class PasswordService {
public:
    static std::string hash(const std::string& password) {
        return BCrypt::generateHash(password);
    }
    
    static bool verify(const std::string& password, const std::string& hash) {
        return BCrypt::validatePassword(password, hash);
    }
};