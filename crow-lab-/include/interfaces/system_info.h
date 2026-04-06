#pragma once 

#include <string>


namespace interfaces{

class ISystemInfo{
public:
~ISystemInfo() = default;

virtual std::string getCompilerVersion() const = 0;
virtual std::string getCppStandard() const = 0;
virtual std::string getOsInfo() const = 0;
virtual std::string getBuildType() const = 0;

    };
}