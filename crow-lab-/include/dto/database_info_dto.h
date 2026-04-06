#pragma once

#include <string>
#include <crow.h>

namespace dto{

    class DatabaseInfoDto {

    std::string driver; // тип пд 
    std::string database_name; 
    std::string status;
    std::string version;

    public:
    
    DatabaseInfoDto() = default; 

    DatabaseInfoDto(const std::string& driv,
    const std::string& database,
    const std::string& stat,
    const std::string& vers
    ) : driver(driv), database_name(database), status(stat), version(vers) 
    { }

    crow::json::wvalue toJson(){

    crow::json::wvalue json;

    json["driver"] = driver;
    json["database_name"] = database_name;
    json["status"] = status;
    json["version"] = version;

    return json;

    }
    }; // class database info dto
} // namespace std 