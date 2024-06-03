#pragma once

#include <Networking/NetworkManager.hpp>
#include <DiscordAuth.hpp>

class Core {
    public :
        Core();
        ~Core();
        void Initialize();
    private :
        Networking::NetworkManager *m_pNetworkManager;
        DiscordAuth *m_pDiscordAuth;

};