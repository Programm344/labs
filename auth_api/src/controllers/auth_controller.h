#pragma once
#include <crow.h>
#include <memory>
#include "../services/token_service.h"
#include "../database/user_repository.h"

class AuthController {
private:
// вспомогательные ф-ции
 std::shared_ptr<TokenService> token_service_;
 std::string extract_token(const crow::request& req);
 std::string hash_password(const std::string& password);
 bool verify_password(const std::string& password, const std::string& hash);

 public:
    explicit AuthController(std::shared_ptr<TokenService> token_service);


    // API
    crow::response register_user(const crow::request& req);
    crow::response login(const crow::request& req);
    crow::response get_me(const crow::request& req);
    crow::response logout(const crow::request& req);
    crow::response get_tokens(const crow::request& req);
    crow::response logout_all(const crow::request& req);
    crow::response refresh_token(const crow::request& req);
    crow::response change_password(const crow::request& req);


};