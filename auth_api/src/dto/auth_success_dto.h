#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "user_dto.h"

using json = nlohmann::json;

class AuthSuccessDTO{
public:
const std::string accessToken;
const std::string refreshToken;
const UserDTO user;

 AuthSuccessDTO(const std::string& accessToken,
                   const std::string& refreshToken,
                   const UserDTO& user)
        : accessToken(accessToken)
        , refreshToken(refreshToken)
        , user(user)
    {}
     json toJson() const {
        return {
            {"access_token", accessToken},
            {"refresh_token", refreshToken},
            {"user", user.toJson()}
        };
    }
};