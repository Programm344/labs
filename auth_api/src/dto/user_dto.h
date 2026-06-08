#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dto {

class UserDTO {
public:
    const int id;
    const std::string username;
    const std::string email;
    const std::string birthday;
    
    UserDTO(int id, const std::string& username, 
            const std::string& email, const std::string& birthday)
        : id(id), username(username), email(email), birthday(birthday) {}
    
    json to_json() const {
        return {
            {"id", id},
            {"username", username},
            {"email", email},
            {"birthday", birthday}
        };
    }
};

} // namespace dto