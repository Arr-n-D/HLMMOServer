#pragma once

#include <DiscordAuth.hpp>
#include <Networking/NetworkManager.hpp>

class Core {
   public:
    Core();
    ~Core();
    void Initialize();
    std::string GetEnvironmentVariable( std::string environmentVariableName );

   private:
    Networking::NetworkManager *m_pNetworkManager;
    DiscordAuth *m_pDiscordAuth;
};
