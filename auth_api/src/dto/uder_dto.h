#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class UserDTO {
    public:
    const int id;
    const std::string username;
    const std::string email;
    conts std::string birthday;

    UserDTO(const int id,
    const std::string username,
    const std::string email,
    conts std::string birthday) : id(id), username(username), email(email), birthday(birthday) {}

    json toJson() const {
        return {
             {"id", id},
            {"username", username},
            {"email", email},
            {"birthday", birthday}
        };
    }
};