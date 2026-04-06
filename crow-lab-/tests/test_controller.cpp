#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <crow.h>
#include "../include/controllers/info_controller.h"
#include "../include/dto/server_info_dto.h"
#include "../include/dto/client_info_dto.h"
#include "../include/dto/database_info_dto.h"
#include "mocks/mock_system_info.h"
#include "mocks/mock_client_data.h"

using namespace testing;
using namespace mocks;


// ServerInfo тесты

class ServerInfoTest : public Test {
protected:
    void SetUp() override {
        mockSystem = std::make_shared<MockSystemInfo>();
        mockClient = std::make_shared<MockClientData>();
        controller = std::make_shared<controllers::InfoController>(mockSystem, mockClient);
    }
    std::shared_ptr<MockSystemInfo> mockSystem;
    std::shared_ptr<MockClientData> mockClient;
    std::shared_ptr<controllers::InfoController> controller;
};

TEST_F(ServerInfoTest, ReturnsCorrectCompilerVersion) {
    
    // НАСТРОЙКА ВЫЗОВА ФУНКЦИИ

    // проверка на один вызов ф-ции
    EXPECT_CALL(*mockSystem, getCompilerVersion())
        .WillOnce(Return("GCC 11.4.0"));
    EXPECT_CALL(*mockSystem, getCppStandard())
        .WillOnce(Return("C++17"));
    EXPECT_CALL(*mockSystem, getOsInfo())
        .WillOnce(Return("Ubuntu 22.04"));
    EXPECT_CALL(*mockSystem, getBuildType())
        .WillOnce(Return("Debug"));
    
        //ПУСТОЙ ЗАПРОС 
    crow::request req;
    
 
    auto res = controller->serverInfo(req);
    auto body = res.body;
    
   // ПРОВЕРКА ОТВЕТА НА СООТВЕТСТИВЕ

    EXPECT_EQ(res.code, 200);
    EXPECT_NE(body.find("GCC 11.4.0"), std::string::npos);
    EXPECT_NE(body.find("C++17"), std::string::npos);
    EXPECT_NE(body.find("Ubuntu 22.04"), std::string::npos);
    EXPECT_NE(body.find("Debug"), std::string::npos);

  
    EXPECT_EQ(res.get_header_value("Content-Type"), "application/json");
}


TEST_F(ServerInfoTest, ReturnsCorrectLocaleAndTimezone) {
   
    EXPECT_CALL(*mockSystem, getCompilerVersion()).WillOnce(Return("GCC"));
    EXPECT_CALL(*mockSystem, getCppStandard()).WillOnce(Return("C++17"));
    EXPECT_CALL(*mockSystem, getOsInfo()).WillOnce(Return("Ubuntu"));
    EXPECT_CALL(*mockSystem, getBuildType()).WillOnce(Return("Debug"));
    
    crow::request req;
    
    
    auto res = controller->serverInfo(req);
    auto body = res.body;
    
    
    EXPECT_NE(body.find("\"locale\":\"ru\""), std::string::npos);
    EXPECT_NE(body.find("\"timezone\":\"Moscow\""), std::string::npos);
}

TEST_F(ServerInfoTest, ReturnsValidJson) {
  

    EXPECT_CALL(*mockSystem, getCompilerVersion()).WillOnce(Return("GCC"));
    EXPECT_CALL(*mockSystem, getCppStandard()).WillOnce(Return("C++17"));
    EXPECT_CALL(*mockSystem, getOsInfo()).WillOnce(Return("Ubuntu"));
    EXPECT_CALL(*mockSystem, getBuildType()).WillOnce(Return("Debug"));
    
    crow::request req;
    
    auto res = controller->serverInfo(req);
    auto body = res.body;
    
   
    EXPECT_TRUE(body.front() == '{');
    EXPECT_TRUE(body.back() == '}');
    EXPECT_NE(body.find("\"compiler_version\""), std::string::npos);
    EXPECT_NE(body.find("\"cpp_standard\""), std::string::npos);
    EXPECT_NE(body.find("\"operating_system\""), std::string::npos);
    EXPECT_NE(body.find("\"build_type\""), std::string::npos);
    EXPECT_NE(body.find("\"locale\""), std::string::npos);
    EXPECT_NE(body.find("\"timezone\""), std::string::npos);
}

// client info тесты 

class ClientInfoTest : public Test {
protected:
    void SetUp() override {
        mockSystem = std::make_shared<MockSystemInfo>();
        mockClient = std::make_shared<MockClientData>();
        controller = std::make_shared<controllers::InfoController>(mockSystem, mockClient);
    }
    
    //параметры по умолчанию, чтобы тестировать отдельно параметры
    crow::request createRequest(const std::string& ip = "127.0.0.1",
                                 const std::string& ua = "curl/7.68.0",
                                 const std::string& lang = "ru-RU") {
        crow::request req;
        req.remote_ip_address = ip;
        req.add_header("User-Agent", ua);
        req.add_header("Accept-Language", lang);
        return req;
    }
    
    std::shared_ptr<MockSystemInfo> mockSystem;
    std::shared_ptr<MockClientData> mockClient;
    std::shared_ptr<controllers::InfoController> controller;
};

TEST_F(ClientInfoTest, ReturnsCorrectIpAddress) {
  
    EXPECT_CALL(*mockClient, getIp(_)).WillOnce(Return("192.168.1.100"));
    EXPECT_CALL(*mockClient, getUserAgent(_)).WillOnce(Return("curl/7.68.0"));
    EXPECT_CALL(*mockClient, getMethod(_)).WillOnce(Return("GET"));
    EXPECT_CALL(*mockClient, getLanguage(_)).WillOnce(Return("ru-RU"));
    
    crow::request req = createRequest("192.168.1.100");
    
    
    auto res = controller->clientInfo(req);
    auto body = res.body;
    
   
    EXPECT_EQ(res.code, 200);
    EXPECT_NE(body.find("192.168.1.100"), std::string::npos);
}

TEST_F(ClientInfoTest, ControllerExtractsUserAgentFromClientData) {

   
    std::string expectedUa = "Mozilla/5.0 (Windows NT 10.0; Win64; x64)";
    EXPECT_CALL(*mockClient, getUserAgent(_))
        .WillOnce(Return(expectedUa));
    EXPECT_CALL(*mockClient, getIp(_)).WillOnce(Return("127.0.0.1"));
    EXPECT_CALL(*mockClient, getMethod(_)).WillOnce(Return("GET"));
    EXPECT_CALL(*mockClient, getLanguage(_)).WillOnce(Return("ru-RU"));

    crow::request req;
    auto res = controller->clientInfo(req);

        // ищет текст в строке с json текстом 
       EXPECT_NE(res.body.find(expectedUa), std::string::npos);
    
   
}

TEST_F(ClientInfoTest, ReturnsCorrectRequestMethod) {
    
    EXPECT_CALL(*mockClient, getIp(_)).WillOnce(Return("127.0.0.1"));
    EXPECT_CALL(*mockClient, getUserAgent(_)).WillOnce(Return("curl"));
    EXPECT_CALL(*mockClient, getMethod(_)).WillOnce(Return("POST"));
    EXPECT_CALL(*mockClient, getLanguage(_)).WillOnce(Return("ru-RU"));
    
  
    crow::request req;
    
  
    auto res = controller->clientInfo(req);
    auto body = res.body;
    
    
    EXPECT_NE(body.find("POST"), std::string::npos);
}


// database тесты

class DatabaseInfoTest : public Test {
protected:
    void SetUp() override {
        mockSystem = std::make_shared<MockSystemInfo>();
        mockClient = std::make_shared<MockClientData>();
        controller = std::make_shared<controllers::InfoController>(mockSystem, mockClient);
    }
    
    std::shared_ptr<MockSystemInfo> mockSystem;
    std::shared_ptr<MockClientData> mockClient;
    std::shared_ptr<controllers::InfoController> controller;
};

TEST_F(DatabaseInfoTest, ReturnsCorrectDatabaseInfo) {
 
    crow::request req;
    
   
    auto res = controller->databaseInfo(req);
    auto body = res.body;
    

    EXPECT_EQ(res.code, 200);
    EXPECT_NE(body.find("SQLite"), std::string::npos);
    EXPECT_NE(body.find("3.40.0"), std::string::npos);
    EXPECT_NE(body.find("in-memory"), std::string::npos);
    EXPECT_NE(body.find("Connected"), std::string::npos);
    EXPECT_EQ(res.get_header_value("Content-Type"), "application/json");
}


// DTO тесты

TEST(DtoTest, ServerInfoDtoConvertsToJson) {
    dto::ServerInfoDto dto("GCC 11.4", "C++17", "1.0", "Ubuntu", "Debug", "ru", "Europe/Moscow");
    auto json = dto.toJson();
   
    SUCCEED();
}

TEST(DtoTest, ClientInfoDtoConvertsToJson) {
    dto::ClientInfoDto dto("192.168.1.1", "curl", "GET", "ru-RU");
    auto json = dto.toJson();
    SUCCEED();
}

TEST(DtoTest, DatabaseInfoDtoConvertsToJson) {
    dto::DatabaseInfoDto dto("SQLite", "3.40.0", "test.db", "Connected");
    auto json = dto.toJson();
    SUCCEED();
}