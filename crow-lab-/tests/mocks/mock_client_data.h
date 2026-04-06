#ifndef MOCK_CLIENT_DATA_H
#define MOCK_CLIENT_DATA_H

#include <gmock/gmock.h>
#include "../include/interfaces/client_data.h"

namespace mocks {

class MockClientData : public interfaces::IClientData {
public:
    MOCK_METHOD(std::string, getIp, (const crow::request&), (const, override));
    MOCK_METHOD(std::string, getUserAgent, (const crow::request&), (const, override));
    MOCK_METHOD(std::string, getMethod, (const crow::request&), (const, override));
    MOCK_METHOD(std::string, getLanguage, (const crow::request&), (const, override));
};
}
#endif