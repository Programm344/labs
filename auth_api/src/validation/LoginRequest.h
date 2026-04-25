#pragma once
#include <string>
#include <vector>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


class LoginRequest {
private:
    std::string username;
    std::string password;
    std::vector<std::string> errors;
    
    bool validateUsername() {
        // Только латинские буквы, начинается с заглавной, мин 7 символов
        std::regex pattern("^[A-Z][a-zA-Z]{6,}$");
        return std::regex_match(username, pattern);
    }
    
    bool validatePassword() {
        if (password.length() < 8) return false;
        
        bool hasDigit = false, hasSpecial = false, hasUpper = false, hasLower = false;
        for (char c : password) {
            if (isdigit(c)) hasDigit = true;
            else if (isupper(c)) hasUpper = true;
            else if (islower(c)) hasLower = true;
            else if (!isalnum(c)) hasSpecial = true;
        }
        return hasDigit && hasSpecial && hasUpper && hasLower;
    }
    
public:
    LoginRequest(const json& data) {
        if (data.contains("username")) username = data["username"];
        if (data.contains("password")) password = data["password"];
    }
    
    bool validate() {
        errors.clear();
        if (!validateUsername()) {
            errors.push_back("Username must start with uppercase letter and be at least 7 chars");
        }
        if (!validatePassword()) {
            errors.push_back("Password must be at least 8 chars with digit, special char, uppercase and lowercase");
        }
        return errors.empty();
    }
    
    const std::vector<std::string>& getErrors() const { return errors; }
    
    struct LoginData {
        std::string username;
        std::string password;
    };
    
    LoginData toDTO() const { return {username, password}; }
    
    std::string getUsername() const { return username; }
    std::string getPassword() const { return password; }
};