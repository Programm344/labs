#ifndef SERVER_INFO_DTO_H
#define SERVER_INFO_DTO_H

#include <string>
#include <crow.h>

namespace dto {
    class ServerInfoDto{
        std::string compiler_version;
        std::string cpp_standard;
        std::string framework_version;
        std::string operating_system;
        std::string build_type;
        std::string locale;
        std::string timezone;
    public:
        ServerInfoDto() = default;

        ServerInfoDto(const std::string& compiler,
        const std::string& standart,
        const std::string& framework,
        const std::string& os,
        const std::string& build,
        const std::string& lc,
        const std::string& tz) : compiler_version(compiler), cpp_standard(standart), framework_version(framework), operating_system(os), build_type(build), locale(lc), timezone(tz) 
        {  }

        crow::json::wvalue toJson() const { // сериализация инфы о сервере 
            crow::json::wvalue json;

            json["compiler_version"] = compiler_version;
            json["cpp_standard"] = cpp_standard;
            json["framework_version"] = framework_version;
            json["operating_system"] = operating_system;
            json["build_type"] = build_type;
            json["locale"] = locale;
            json["timezone"] = timezone;

            return json;


        }
    }; // класс
} // namespace dto
#endif 