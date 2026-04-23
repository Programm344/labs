#pragma once
#include <string>

namespace interfaces {

class IDatabaseInfo {
public:
    virtual ~IDatabaseInfo() = default;
    
    virtual std::string getDriver() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDatabaseName() const = 0;
    virtual std::string getStatus() const = 0;
};

} // namespace interfaces