
#include "auth_controller.h"
#include "../validation/login_request.h"
#include "../validation/register_request.h"
#include "../dto/user_dto.h"
#include "../dto/auth_success_dto.h"
#include "../dto/token_list_dto.h"
#include <bcrypt/BCrypt.hpp>



AuthController::AuthController(std::shared_ptr<TokenService> token_service)
    : token_service_(token_service) {}

    std::string AuthController::extract_token(const crow::request& req){
         std::string auth_header = req.get_header_value("Authorization");
         if (auth_header.substr(0, 7) == "Bearer ") {
            return auth_header.substr(7);
        }
         return "";
    }

