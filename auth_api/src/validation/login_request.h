#pragma once
#include <string>
#include <vector>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class LoginRequest {
private:
    std::string username_;
    std::string password_;
    std::vector<std::string> errors_;
    
    bool validate_username() {
        // Только латинские буквы, начинается с заглавной, мин 7 символов
       std::regex pattern("^[A-Z][a-zA-Z0-9]{6,}$");
        return std::regex_match(username_, pattern);
    }
    
    bool validate_password() {
        if (password_.length() < 8) return false;
        
        bool has_digit = false, has_special = false, has_upper = false, has_lower = false;
        for (char c : password_) {
            if (isdigit(c)) has_digit = true;
            else if (isupper(c)) has_upper = true;
            else if (islower(c)) has_lower = true;
            else if (!isalnum(c)) has_special = true;
        }
        return has_digit && has_special && has_upper && has_lower;
    }
    
public:
    LoginRequest(const json& data) {
        if (data.contains("username")) username_ = data["username"];
        if (data.contains("password")) password_ = data["password"];
    }
    
    bool validate() {
        errors_.clear();
        if (!validate_username()) {
            errors_.push_back("Username must start with uppercase letter and be at least 7 chars");
            return false;
        }
        if (!validate_password()) {
            errors_.push_back("Password must be at least 8 chars with digit, special char, uppercase and lowercase");
            return false;
        }
        return true;
    }
    
    const std::vector<std::string>& get_errors() const { return errors_; }
    
    struct LoginData {
        std::string username;
        std::string password;
    };
    
    LoginData to_dto() const { return {username_, password_}; }
    
    std::string get_username() const { return username_; }
    std::string get_password() const { return password_; }
};