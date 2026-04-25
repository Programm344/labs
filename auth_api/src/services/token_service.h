#pragma once

#include <string> 
#include <optional>  
#include <vector>

struct ActiveTokenInfo {
    std::string id;         // ID токена (первые 16 символов хеша)
    std::string createdAt;  // Дата создания
    std::string expiresAt;  // Дата истечения
    std::string lastUsed;   // Последнее использование
};

class TokenService {
public:
    // возвращается при логине
    struct TokenPair {
        std::string accessToken;   
        std::string refreshToken;  
    };

      TokenService();
      TokenPair generateTokens(int userId);
      bool validateAccessToken(const std::string& token);
      bool validateRefreshToken(const std::string& token);
      int getUserIdFromToken(const std::string& token);
      void revokeToken(const std::string& token);
      void revokeAllUserTokens(int userId);
      std::vector<ActiveTokenInfo> getUserActiveTokens(int userId);
      std::optional<TokenPair> refreshTokens(const std::string& refreshToken);
      void enforceTokenLimit(int userId);
private:
    // читается из .env
    std::string secretKey;      
    int accessTokenTTL;        
    int refreshTokenTTL;        
    int maxActiveTokens;        

    // создает хеш токена
    std::string hashToken(const std::string& token);

    //сохраняет хеш токена в бд
    void storeTokenHash(const std::string& tokenHash, int userId, int ttlMinutes, bool isRefresh);
    bool isTokenRevoked(const std::string& tokenHash);
    void markTokenAsRevoked(const std::string& tokenHash, int userId);