#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "user_dto.h"

using json = nlohmann::json;

namespace dto {

class AuthSuccessDTO {
public:
    const std::string access_token;
    const std::string refresh_token;
    const UserDTO user;
    
    AuthSuccessDTO(const std::string& access_token, 
                   const std::string& refresh_token,
                   const UserDTO& user)
        : access_token(access_token), refresh_token(refresh_token), user(user) {}
    
    json to_json() const {
        return {
            {"access_token", access_token},
            {"refresh_token", refresh_token},
            {"user", user.to_json()}
        };
    }
};

} 