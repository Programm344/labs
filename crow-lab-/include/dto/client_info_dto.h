#pragma once 

#include <string>
#include <crow.h>

namespace dto{

    class ClientInfoDto{
        std::string ip_address;
        std::string user_agent;
        std::string request_method;
        std::string accept_language;

    public:
        ClientInfoDto() = default;
        ClientInfoDto(const std::string& ip,
        const std::string& ua,
        const std::string& method,
        const std::string& accept
        ) : ip_address(ip), user_agent(ua), request_method(method), accept_language(accept) 
        { }
        
        crow::json::wvalue toJson() const {

        crow::json::wvalue json;
        
        json["ip_address"] = ip_address;
        json["user_agent"] = user_agent;
        json["request_method"] = request_method;
        json["accept_language"] = accept_language;
        
        return json;

        }


    }; //client info dto
} // namespace dto