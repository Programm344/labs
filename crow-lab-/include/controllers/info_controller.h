#pragma once

#ifndef CROW_VERSION
#define CROW_VERSION "1.0.0"
#endif

#include <crow.h>
#include <memory>
#include "../interfaces/system_info.h"
#include "../interfaces/client_data.h"
#include "../dto/server_info_dto.h"
#include "../dto/client_info_dto.h"
#include "../dto/database_info_dto.h"
#include "../config/app_config.h"

namespace controllers{

class InfoController{
private:
std::shared_ptr<interfaces::ISystemInfo> ISystemInfo_;
std::shared_ptr<interfaces::IClientData> IClientData_;

public:

InfoController(std::shared_ptr<interfaces::ISystemInfo> SystemInfo, std::shared_ptr<interfaces::IClientData> ClientData) 
: ISystemInfo_(SystemInfo), IClientData_(ClientData) { }

crow::response serverInfo(const crow::request& req){
    dto::ServerInfoDto dto(
        ISystemInfo_->getCompilerVersion(),
        ISystemInfo_->getCppStandard(),
        CROW_VERSION,
        ISystemInfo_->getOsInfo(),
        ISystemInfo_->getBuildType(),
        config::LOCALE,
        config::TIMEZONE
    );

    crow::response res(dto.toJson());
    res.add_header("Content-Type", "application/json");
    return res;

}

  crow::response clientInfo(const crow::request& req) {
        dto::ClientInfoDto dto(
            IClientData_->getIp(req),
            IClientData_->getUserAgent(req),
            IClientData_->getMethod(req),
            IClientData_->getLanguage(req)
        );
        
        crow::response res(dto.toJson());
        res.add_header("Content-Type", "application/json");
        return res;
    }

      crow::response databaseInfo(const crow::request& req) {
        
        dto::DatabaseInfoDto dto(
            "SQLite",
            "3.40.0",
            "in-memory",
            "Connected"
        );

        crow::response res(dto.toJson());
        res.add_header("Content-Type", "application/json");
        return res;

}
};
}