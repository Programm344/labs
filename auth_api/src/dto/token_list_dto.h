#pragma once
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace dto {

struct TokenInfo {
    std::string id;
    std::string created_at;
    std::string expires_at;
    std::string last_used;
    std::string device;
    std::string ip_address;
    
    json to_json() const {
        return {
            {"id", id},
            {"created_at", created_at},
            {"expires_at", expires_at},
            {"last_used", last_used},
            {"device", device},
            {"ip_address", ip_address}
        };
    }
};

class TokenListDTO {
public:
    std::vector<TokenInfo> tokens;
    
    void add_token(const TokenInfo& token) {
        tokens.push_back(token);
    }
    
    void add_token(const std::string& id, 
                   const std::string& created_at,
                   const std::string& expires_at,
                   const std::string& last_used) {
        TokenInfo info;
        info.id = id;
        info.created_at = created_at;
        info.expires_at = expires_at;
        info.last_used = last_used;
        tokens.push_back(info);
    }
    
    json to_json() const {
        json arr = json::array();
        for (const auto& token : tokens) {
            arr.push_back(token.to_json());
        }
        return arr;
    }
    
    bool is_empty() const { return tokens.empty(); }
    size_t count() const { return tokens.size(); }
};

} // namespace dto