
#pragma once

#include <string>
#include <crow.h>
#include "../interfaces/client_data.h"

namespace services {
class HttpClientData : public interfaces::IClientData {
public:
 std::string getIp(const crow::request& req) const override{

 if (req.headers.count("X-Forwarded-For"))
     return req.get_header_value("X-Forwarded-For");
 return req.remote_ip_address;

 };

 std::string getUserAgent(const crow::request& req) const override{
    return req.get_header_value("User-Agent");
 };
 std::string getMethod(const crow::request& req) const override{
 return crow::method_name(req.method);
 }
 ;
 std::string getLanguage(const crow::request& req) const override{
    return req.get_header_value("Accept-Language");
 };
};
}