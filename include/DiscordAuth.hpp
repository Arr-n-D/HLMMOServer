#include <string>

class DiscordAuth {
    public:
        DiscordAuth(std::string clientId, std::string secret, std::string redirect_uri);
        ~DiscordAuth();
        std::string GetClientId();
        std::string GetSecret();
        std::string GetRedirectUri();
    private:
        std::string clientId;
        std::string secret; 
        std::string redirect_uri;
};

