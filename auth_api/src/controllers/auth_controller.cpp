#include "auth_controller.h"
#include "../validation/login_request.h"
#include "../validation/register_request.h"
#include "../dto/user_dto.h"
#include "../dto/auth_success_dto.h"
#include "../dto/token_list_dto.h"
#include <bcrypt/BCrypt.hpp>

AuthController::AuthController(std::shared_ptr<TokenService> token_service)
    : token_service_(token_service) {}

std::string AuthController::extract_token(const crow::request& req) {
    std::string auth_header = req.get_header_value("Authorization");
    if (auth_header.substr(0, 7) == "Bearer ") {
        return auth_header.substr(7);
    }
    return "";
}

std::string AuthController::hash_password(const std::string& password) {
    return BCrypt::generateHash(password);
}

bool AuthController::verify_password(const std::string& password, const std::string& hash) {
    return BCrypt::validatePassword(password, hash);
}

// 1. РЕГИСТРАЦИЯ
crow::response AuthController::register_user(const crow::request& req) {
    auto body = json::parse(req.body);
    if (body.is_discarded()) {
        return crow::response(400, R"({"error":"Invalid JSON"})");
    }
    
    RegisterRequest request(body);
    if (!request.validate()) {
        json errors;
        errors["errors"] = request.get_errors();
        return crow::response(422, errors.dump());
    }
    
    if (!UserRepository::is_username_unique(request.get_username())) {
        return crow::response(422, R"({"error":"Username already exists"})");
    }
    if (!UserRepository::is_email_unique(request.get_email())) {
        return crow::response(422, R"({"error":"Email already exists"})");
    }
    
    std::string password_hash = hash_password(request.get_password());
    bool created = UserRepository::create(
        request.get_username(),
        request.get_email(),
        password_hash,
        request.get_birthday()
    );
    
    if (!created) {
        return crow::response(500, R"({"error":"Failed to create user"})");
    }
    
    auto user = UserRepository::find_by_username(request.get_username());
    if (!user.has_value()) {
        return crow::response(500, R"({"error":"User created but not found"})");
    }
    
    dto::UserDTO user_dto(user->id, user->username, user->email, user->birthday);
    
    crow::response res(201);
    res.set_header("Content-Type", "application/json");
    res.body = user_dto.to_json().dump();
    return res;
}

// 2. ЛОГИН
crow::response AuthController::login(const crow::request& req) {
    auto body = json::parse(req.body);
    if (body.is_discarded()) {
        return crow::response(400, R"({"error":"Invalid JSON"})");
    }
    
    LoginRequest request(body);
    if (!request.validate()) {
        json errors;
        errors["errors"] = request.get_errors();
        return crow::response(422, errors.dump());
    }
    
    auto user = UserRepository::find_by_username(request.get_username());
    if (!user.has_value()) {
        return crow::response(401, R"({"error":"Invalid credentials"})");
    }
    
    if (!verify_password(request.get_password(), user->password_hash)) {
        return crow::response(401, R"({"error":"Invalid credentials"})");
    }
    
    auto tokens = token_service_->generate_tokens(user->id);
    
    dto::UserDTO user_dto(user->id, user->username, user->email, user->birthday);
    dto::AuthSuccessDTO success_dto(tokens.access_token, tokens.refresh_token, user_dto);
    
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = success_dto.to_json().dump();
    return res;
}

// 3. ПОЛУЧЕНИЕ ИНФОРМАЦИИ О СЕБЕ
crow::response AuthController::get_me(const crow::request& req) {
    std::string token = extract_token(req);
    if (token.empty()) {
        return crow::response(401, R"({"error":"Token required"})");
    }
    
    if (!token_service_->validate_access_token(token)) {
        std::cout << "[DEBUG] get_me: invalid token" << std::endl;
        return crow::response(401, R"({"error":"Invalid or expired token"})");
    }

    int user_id = token_service_->get_user_id_from_token(token);
    if (user_id == -1) {
        return crow::response(401, R"({"error":"Invalid token"})");
    }
    
    auto user = UserRepository::find_by_id(user_id);
    if (!user.has_value()) {
        return crow::response(404, R"({"error":"User not found"})");
    }

    
    
    dto::UserDTO user_dto(user->id, user->username, user->email, user->birthday);
    
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = user_dto.to_json().dump();
    return res;
}

// 4. ВЫХОД
crow::response AuthController::logout(const crow::request& req) {
    std::string token = extract_token(req);
    if (token.empty()) {
        return crow::response(401, R"({"error":"Token required"})");
    }
    
    token_service_->revoke_token(token);
    return crow::response(200, R"({"message":"Logged out successfully"})");
}

// 5. СПИСОК ТОКЕНОВ
crow::response AuthController::get_tokens(const crow::request& req) {
    std::string token = extract_token(req);
    if (token.empty()) {
        return crow::response(401, R"({"error":"Token required"})");
    }
    
    int user_id = token_service_->get_user_id_from_token(token);
    if (user_id == -1) {
        return crow::response(401, R"({"error":"Invalid token"})");
    }
    
    auto active_tokens = token_service_->get_user_active_tokens(user_id);
    
    dto::TokenListDTO token_list;
    for (const auto& t : active_tokens) {
        token_list.add_token(t.id, t.created_at, t.expires_at, t.last_used);
    }
    
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = token_list.to_json().dump();
    return res;
}

// 6. ВЫХОД СО ВСЕХ УСТРОЙСТВ
crow::response AuthController::logout_all(const crow::request& req) {
    std::string token = extract_token(req);
    if (token.empty()) {
        return crow::response(401, R"({"error":"Token required"})");
    }
    
    int user_id = token_service_->get_user_id_from_token(token);
    if (user_id == -1) {
        return crow::response(401, R"({"error":"Invalid token"})");
    }
    
    token_service_->revoke_all_user_tokens(user_id);
    return crow::response(200, R"({"message":"Logged out from all devices"})");
}

// 7. ОБНОВЛЕНИЕ ТОКЕНА
crow::response AuthController::refresh_token(const crow::request& req) {
    auto body = json::parse(req.body);
    if (body.is_discarded() || !body.contains("refresh_token")) {
        return crow::response(400, R"({"error":"refresh_token required"})");
    }
    
    std::string refresh_token = body["refresh_token"];
    auto new_tokens = token_service_->refresh_tokens(refresh_token);
    
    if (!new_tokens.has_value()) {
        return crow::response(401, R"({"error":"Invalid or expired refresh token"})");
    }
    
    int user_id = token_service_->get_user_id_from_token(new_tokens->access_token);
    auto user = UserRepository::find_by_id(user_id);
    if (!user.has_value()) {
        return crow::response(404, R"({"error":"User not found"})");
    }
    
    dto::UserDTO user_dto(user->id, user->username, user->email, user->birthday);
    dto::AuthSuccessDTO success_dto(new_tokens->access_token, new_tokens->refresh_token, user_dto);
    
    crow::response res(200);
    res.set_header("Content-Type", "application/json");
    res.body = success_dto.to_json().dump();
    return res;
}

// 8. ИЗМЕНЕНИЕ ПАРОЛЯ
crow::response AuthController::change_password(const crow::request& req) {
    auto body = json::parse(req.body);
    if (body.is_discarded()) {
        return crow::response(400, R"({"error":"Invalid JSON"})");
    }
    
    if (!body.contains("current_password") || !body.contains("new_password") || 
        !body.contains("confirm_password")) {
        return crow::response(400, R"({"error":"current_password, new_password and confirm_password required"})");
    }
    
    std::string current_password = body["current_password"];
    std::string new_password = body["new_password"];
    std::string confirm_password = body["confirm_password"];
    
    std::string token = extract_token(req);
    if (token.empty()) {
        return crow::response(401, R"({"error":"Token required"})");
    }
    
    int user_id = token_service_->get_user_id_from_token(token);
    if (user_id == -1) {
        return crow::response(401, R"({"error":"Invalid token"})");
    }
    
    auto user = UserRepository::find_by_id(user_id);
    if (!user.has_value()) {
        return crow::response(404, R"({"error":"User not found"})");
    }
    
    if (!verify_password(current_password, user->password_hash)) {
        return crow::response(401, R"({"error":"Current password is incorrect"})");
    }
    
    if (new_password != confirm_password) {
        return crow::response(422, R"({"error":"New passwords do not match"})");
    }
    
    // Проверка сложности нового пароля
    if (new_password.length() < 8) {
        return crow::response(422, R"({"error":"Password too weak"})");
    }
    
    std::string new_hash = hash_password(new_password);
    if (!UserRepository::update_password(user_id, new_hash)) {
        return crow::response(500, R"({"error":"Failed to update password"})");
    }
    
    token_service_->revoke_all_user_tokens(user_id);
    
    return crow::response(200, R"({"message":"Password changed successfully. Please login again."})");
}