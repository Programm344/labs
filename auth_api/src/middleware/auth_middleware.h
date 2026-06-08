#pragma once
#include <crow.h>
#include <memory>
#include "../services/token_service.h"

class AuthMiddleware{
    std::shared_ptr<TokenService> token_service_;
public:
 AuthMiddleware(std::shared_ptr<TokenService> token_service) : token_service_(token_service) {}

struct context 
{
 int user_id = -1;
};
    void before_handle (crow::request& req, crow::response& res, context& ctx) {
        std::string auth_header = req.get_header_value("Authorization");
        if (auth_header.empty() ){
            res.code = 401;
            res.body = R"({"error":"Authorization header required"})";
            res.end();
            return;
        } 
         if (auth_header.substr(0,7) != "Bearer") {
            res.code = 401;
            res.body = R"({"error":"Invalid authorization format. Use Bearer <token>"})";
            res.end();
            return;
        } 

        std::string token = auth_header.substr(7);

        if (!token_service_->validate_access_token(token)){
            res.code = 401;
            res.body = R"({"error":"Invalid or expired token"})";
            res.end();
            return;
        }

        ctx.user_id = token_service_->get_user_id_from_token(token);
        req.add_header("X-User-Id", std::to_string(ctx.user_id));
         }
         void after_handle(crow::request& req, crow::response& res, context& ctx) {
     
       }
    };
