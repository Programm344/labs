#ifndef MOCK_SYSTEM_INFO_H
#define MOCK_SYSTEM_INFO_H


#include <gmock/gmock.h>
#include "../include/interfaces/system_info.h"

namespace mocks {

class MockSystemInfo : public interfaces::ISystemInfo {
public:
    MOCK_METHOD(std::string, getCompilerVersion, (), (const, override));
    MOCK_METHOD(std::string, getCppStandard, (), (const, override));
    MOCK_METHOD(std::string, getOsInfo, (), (const, override));
    MOCK_METHOD(std::string, getBuildType, (), (const, override));
};
}
#endif