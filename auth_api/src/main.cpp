#include <crow.h>
#include <memory>
#include <iostream>
#include <cstdlib>
#include "database/database.h"
#include "database/user_repository.h"
#include "services/token_service.h"
#include "controllers/auth_controller.h"

void load_env() {
    FILE* file = fopen(".env", "r");
    if (!file) {
        std::cout << "No .env file found, using defaults" << std::endl;
        return;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        char* equals = strchr(line, '=');
        if (equals) {
            *equals = '\0';
            std::string key = line;
            std::string value = equals + 1;
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
    fclose(file);
    std::cout << "Environment loaded" << std::endl;
}

int main() {
    std::cout << "Auth API Server" << std::endl;
    load_env();

    if (!Database::get_instance().init()) {
        std::cerr << "Failed to initialize database!" << std::endl;
        return 1;
    }

    auto token_service = std::make_shared<TokenService>();
    auto auth_controller = std::make_shared<AuthController>(token_service);

    // Приложение
    crow::SimpleApp app;

    // Открытые маршруты (для гостей)
    CROW_ROUTE(app, "/api/auth/register")
        .methods(crow::HTTPMethod::POST)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->register_user(req);
    });

    CROW_ROUTE(app, "/api/auth/login")
        .methods(crow::HTTPMethod::POST)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->login(req);
    });

    // Защищённые маршруты (проверка токена внутри контроллера)
    CROW_ROUTE(app, "/api/auth/me")
        .methods(crow::HTTPMethod::GET)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->get_me(req);
    });

    CROW_ROUTE(app, "/api/auth/out")
        .methods(crow::HTTPMethod::POST)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->logout(req);
    });
    
    CROW_ROUTE(app, "/api/auth/tokens")
        .methods(crow::HTTPMethod::GET)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->get_tokens(req);
    });
    
    CROW_ROUTE(app, "/api/auth/out_all")
        .methods(crow::HTTPMethod::POST)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->logout_all(req);
    });
    
    // Refresh не использует middleware (проверка внутри контроллера)
    CROW_ROUTE(app, "/api/auth/refresh")
        .methods(crow::HTTPMethod::POST)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->refresh_token(req);
    });
    
    CROW_ROUTE(app, "/api/auth/change-password")
        .methods(crow::HTTPMethod::POST)
    ([&auth_controller](const crow::request& req) {
        return auth_controller->change_password(req);
    });

    // Корневой маршрут
    CROW_ROUTE(app, "/")
    ([](){
        return "Auth API is running. Use /api/auth/... endpoints";
    });

    // Запуск сервера
    int port = std::getenv("PORT") ? std::stoi(std::getenv("PORT")) : 8080;

    std::cout << "Server running on port " << port << std::endl;
    std::cout << "Open routes (no token required):" << std::endl;
    std::cout << "  POST   /api/auth/register" << std::endl;
    std::cout << "  POST   /api/auth/login" << std::endl;
    std::cout << "  POST   /api/auth/refresh" << std::endl;
    std::cout << "\nProtected routes (Bearer token required):" << std::endl;
    std::cout << "  GET    /api/auth/me" << std::endl;
    std::cout << "  POST   /api/auth/out" << std::endl;
    std::cout << "  GET    /api/auth/tokens" << std::endl;
    std::cout << "  POST   /api/auth/out_all" << std::endl;
    std::cout << "  POST   /api/auth/change-password" << std::endl;

    app.port(port).multithreaded().run();
    Database::get_instance().close();
    
    return 0;
}