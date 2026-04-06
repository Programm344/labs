#pragma once

#include <string>
#include <crow.h>


namespace interfaces{
class IClientData{
public:
~IClientData() = default;

virtual std::string getIp(const crow::request& req) const = 0;
virtual std::string getUserAgent(const crow::request& req) const = 0;
virtual std::string getMethod(const crow::request& req) const = 0;
virtual std::string getLanguage(const crow::request& req) const = 0;


};
}