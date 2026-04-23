#include <crow.h>
#include <memory>
#include <iostream>
#include "controllers/info_controller.h"
#include "services/linux_system_info.h"
#include "services/http_client_data.h"
#include "config/app_config.h"
#include "services/database_info.h"

int main(){
    crow::SimpleApp app;

    auto systemInfo = std::make_shared<services::LinuxSystemInfo>();
    auto clientData = std::make_shared<services::HttpClientData>();
    auto databaseInfo = std::make_shared<services::SqliteDatabaseInfo>();

    auto controller = std::make_shared<controllers::InfoController>(
        systemInfo, clientData, databaseInfo
    );

    std::cout << "Первая лаба по серверным приложениям" << std::endl;

    std::cout << "Сервер запущен на порту " << config::PORT << std::endl;
    std::cout << "Локаль: " << config::LOCALE << std::endl;
    std::cout << "Time zone: " << config::TIMEZONE << std::endl;

    CROW_ROUTE(app, "/info/server")
    ([controller](const crow::request& req) {
        return controller->serverInfo(req);
    });
    
    CROW_ROUTE(app, "/info/client")
    ([controller](const crow::request& req) {
        return controller->clientInfo(req);
    });
    
    CROW_ROUTE(app, "/info/database")
    ([controller](const crow::request& req) {
        return controller->databaseInfo(req);
    });
    
    CROW_ROUTE(app, "/")
    ([]() {
        return crow::response("Используй /info/server, /info/client, /info/database");
    });

      app.port(config::PORT).multithreaded().run();
    

}