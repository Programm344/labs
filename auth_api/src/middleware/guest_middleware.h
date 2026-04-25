#pragma once
#include <crow.h>
#include <memory>
#include "../services/token_service.h"

class GuestMiddleware {
private:
    std::shared_ptr<TokenService> token_service_;
public:
GuestMiddleware(std::shared_ptr<TokenService> token_service) : token_service_(token_service) {}

struct context {};

void before_handle(crow::request& req, crow::response& res, context& ctx) {

    std::string auth_header = req.get_header_value("Authorization");

    if (auth_header.empty()) {
    return;  // гость, всё ок
    }
}
if (auth_header.substr(0, 7) == "Bearer ") {
    std::string token = auth_header.substr(7);
    if (token_service_->validate_access_token(token)) {
        res.code = 403;
        res.body = R"({"error":"Already authenticated. Cannot access this route."})";
        res.end();
        return;
    }
}
};