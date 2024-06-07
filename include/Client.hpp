#pragma once

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#include <boost/uuid/uuid.hpp> 
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "Networking/network_types.hpp"

class Player;
class DiscordAuth;

class Client {
   public:
    Client( ISteamNetworkingSockets *interface, std::string uuid, HSteamNetConnection hConnection, DiscordAuth *pDiscordAuth );
    ~Client();

    void SetPlayer( Player *pPlayer ) { m_pPlayer = pPlayer; }
    Player *GetPlayer() { return m_pPlayer; }
    std::string GetUuid() { return uuid; }
    void Authenticate();
    void SetAuthenticated( bool auth ) { authenticated = auth; }

   private:
    HSteamNetConnection m_hConnection;
    Player *m_pPlayer;
    std::string uuid;
    DiscordAuth *m_pDiscordAuth;
    ISteamNetworkingSockets *m_pInterface;
    bool SendMessage( Packet packet, uint32 size, int nSendFlags );
    bool authenticated = false;
};