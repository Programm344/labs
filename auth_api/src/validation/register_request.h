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
    std::string username_;
    std::string email_;
    std::string password_;
    std::string c_password_;
    std::string birthday_;
    std::vector<std::string> errors_;
    
    bool validate_username() {
        std::regex pattern("^[A-Z][a-zA-Z0-9]{6,}$");;
        return std::regex_match(username_, pattern);
    }
    
    bool validate_email() {
        std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        return std::regex_match(email_, pattern);
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
    
    bool validate_age() {
        int year, month, day;
        if (sscanf(birthday_.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
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
        if (data.contains("username")) username_ = data["username"];
        if (data.contains("email")) email_ = data["email"];
        if (data.contains("password")) password_ = data["password"];
        if (data.contains("c_password")) c_password_ = data["c_password"];
        if (data.contains("birthday")) birthday_ = data["birthday"];
    }
    
    bool validate() {
        errors_.clear();
        if (!validate_username()) errors_.push_back("Invalid username format");
        if (!validate_email()) errors_.push_back("Invalid email format");
        if (!validate_password()) errors_.push_back("Password too weak");
        if (password_ != c_password_) errors_.push_back("Passwords do not match");
        if (!validate_age()) errors_.push_back("You must be at least 14 years old");
        return errors_.empty();
    }
    
    const std::vector<std::string>& get_errors() const { return errors_; }

    struct RegisterData {
        std::string username;
        std::string email;
        std::string password;
        std::string birthday;
    };
    
    RegisterData to_dto() const { return {username_, email_, password_, birthday_}; }
    
    std::string get_username() const { return username_; }
    std::string get_email() const { return email_; }
    std::string get_password() const { return password_; }
    std::string get_birthday() const { return birthday_; }
};