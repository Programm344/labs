#pragma once 

#include <string>
#include <crow.h>
#include <fstream>
#include "../interfaces/system_info.h"

namespace services {
class LinuxSystemInfo : public interfaces::ISystemInfo {
public:
std::string getCompilerVersion() const override {
#if defined(__GNUC__)
    return "GCC" + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#else 
    return "Unknown compiler";
#endif    

};
std::string getCppStandard() const override{
    if (__cplusplus == 202002L) {
        return "C++ 20";
    }
    else if (__cplusplus == 201703L){
        return "C++ 17";
    }
    else return "C++14 or older";
};
std::string getOsInfo() const override{
#ifdef __linux__ 
std::ifstream file("/etc/os-release");
std::string line;
while (getline(file, line)){
   if (line.find("PRETTY_NAME=") != std::string::npos) {
    std::string name = line.substr(line.find('=') + 1);
    if (name.front() == '"' && name.back() == '"'){
        name = name.substr(1, name.length() - 2);
    }
    return name;
   }
}
   return "Linux";
 #else
        return "Unknown OS";
#endif   

}


std::string getBuildType() const override{
    #ifdef NDEBUG
        return "Release";
    #else 
        return "Debug";
    #endif        
}

};
}
