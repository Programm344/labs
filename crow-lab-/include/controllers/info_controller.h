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
#include "../interfaces/i_database_info.h"

namespace controllers{

class InfoController{
private:
std::shared_ptr<interfaces::ISystemInfo> ISystemInfo_;
std::shared_ptr<interfaces::IClientData> IClientData_;
std::shared_ptr<interfaces::IDatabaseInfo> IDatabaseInfo_; 

public:

InfoController(std::shared_ptr<interfaces::ISystemInfo> SystemInfo, std::shared_ptr<interfaces::IClientData> ClientData, std::shared_ptr<interfaces::IDatabaseInfo> DatabaseInfo) 
: ISystemInfo_(SystemInfo), IClientData_(ClientData),  IDatabaseInfo_(DatabaseInfo)  { }

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
        IDatabaseInfo_->getDriver(),
        IDatabaseInfo_->getVersion(),
        IDatabaseInfo_->getDatabaseName(),
        IDatabaseInfo_->getStatus()
    );

    crow::response res(dto.toJson());
    res.add_header("Content-Type", "application/json");
    return res;
}
};
}