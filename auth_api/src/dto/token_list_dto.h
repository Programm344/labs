#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann:json;


struct TokenInfo{
    const std::string id;
    const std::string createdAt;
    const std::string expiresAt;
    const std::string lastUsed;
    const std::string device;
    const std::string IpAddress;

    json toJson() const {
        return {
            {"id", id},
            {"created_at", createdAt},
            {"expires_at", expiresAt},
            {"last_used", lastUsed},
            {"device", device},
            {"ip_address", ipAddress}
        };
    }
};

class TokenListDTO {
public:
std::vector<TokenInfo> tokens;

void addToken(const TokenInfo& token) {
    tokens.push_back(token);
}

void addToken(const std::string& id, 
                  const std::string& createdAt,
                  const std::string& expiresAt,
                  const std::string& lastUsed) {
        TokenInfo info;
        info.id = id;
        info.createdAt = createdAt;
        info.expiresAt = expiresAt;
        info.lastUsed = lastUsed;
        tokens.push_back(info);
    }

// массив токенов
json toJson() const {
json arr = json::array();
for (const auto& token : tokens) {
arr.push_back(token.toJson());
}
return arr;
}

bool isEmpty() const{
return tokens.empty();
}

size_t count() const {
    return tokens.size();
}
};