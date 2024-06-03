#pragma once
#include <format>
#include <string>
#include <Client.hpp>

class DiscordAuth {
    public:
        DiscordAuth(std::string clientId, std::string secret, std::string redirect_uri);
        ~DiscordAuth();
        std::string GetClientId();
        std::string GetSecret();
        std::string GetRedirectUri();
        std::string BuildAuthUrlForClient(Client *client);
    private:
        std::string clientId;
        std::string secret; 
        std::string redirect_uri;
};

