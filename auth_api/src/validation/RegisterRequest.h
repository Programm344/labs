#pragma once
#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class RegisterRequest {
private:
    std::string username;
    std::string email;
    std::string password;
    std::string c_password;
    std::string birthday;
    std::vector<std::string> errors;
    
    bool validateUsername() {
        std::regex pattern("^[A-Z][a-zA-Z]{6,}$");
        return std::regex_match(username, pattern);
    }
    
    bool validateEmail() {
        std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        return std::regex_match(email, pattern);
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
    
    bool validateAge() {
        int year, month, day;
        if (sscanf(birthday.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
            return false;
        }
        
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        
        int age = now_tm->tm_year + 1900 - year;
        if (now_tm->tm_mon + 1 < month || 
            (now_tm->tm_mon + 1 == month && now_tm->tm_mday < day)) {
            age--;
        }
        return age >= 14;
    }
    
public:
    RegisterRequest(const json& data) {
        if (data.contains("username")) username = data["username"];
        if (data.contains("email")) email = data["email"];
        if (data.contains("password")) password = data["password"];
        if (data.contains("c_password")) c_password = data["c_password"];
        if (data.contains("birthday")) birthday = data["birthday"];
    }
    
    bool validate() {
        errors.clear();
        if (!validateUsername()) errors.push_back("Invalid username format");
        if (!validateEmail()) errors.push_back("Invalid email format");
        if (!validatePassword()) errors.push_back("Password too weak");
        if (password != c_password) errors.push_back("Passwords do not match");
        if (!validateAge()) errors.push_back("You must be at least 14 years old");
        return errors.empty();
    }
    
    const std::vector<std::string>& getErrors() const { return errors; }

    struct RegisterData {
        std::string username;
        std::string email;
        std::string password;
        std::string birthday;
    };
    
    RegisterData toDTO() const { return {username, email, password, birthday}; }
    
    std::string getUsername() const { return username; }
    std::string getEmail() const { return email; }
    std::string getPassword() const { return password; }
    std::string getBirthday() const { return birthday; }
};