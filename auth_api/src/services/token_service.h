#pragma once

#include <string> 
#include <optional>  
#include <vector>

struct ActiveTokenInfo {
    std::string id;          // ID токена (первые 16 символов хеша)
    std::string created_at;  // Дата создания
    std::string expires_at;  // Дата истечения
    std::string last_used;   // Последнее использование
};

class TokenService {
public:
    // возвращается при логине
    struct TokenPair {
        std::string access_token;   
        std::string refresh_token;  
    };

    TokenService();
    TokenPair generate_tokens(int user_id);
    bool validate_access_token(const std::string& token);
    bool validate_refresh_token(const std::string& token);
    int get_user_id_from_token(const std::string& token);
    void revoke_token(const std::string& token);
    void revoke_all_user_tokens(int user_id);
    std::vector<ActiveTokenInfo> get_user_active_tokens(int user_id);
    std::optional<TokenPair> refresh_tokens(const std::string& refresh_token);
    void enforce_token_limit(int user_id);
    
private:
    // читается из .env
    std::string secret_key_;      
    int access_token_ttl_;        
    int refresh_token_ttl_;        
    int max_active_tokens_;        

    // создает хеш токена
    std::string hash_token(const std::string& token);

    // сохраняет хеш токена в бд
    void store_token_hash(const std::string& token_hash, int user_id, int ttl_minutes, bool is_refresh);
    bool is_token_revoked(const std::string& token_hash);
    void mark_token_as_revoked(const std::string& token_hash, int user_id);
};