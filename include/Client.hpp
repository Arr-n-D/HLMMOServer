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
   private:
    HSteamNetConnection m_hConnection;
    Player *m_pPlayer;
    std::string uuid;
    DiscordAuth *m_pDiscordAuth;
    ISteamNetworkingSockets *m_pInterface;

   public:
    Client( ISteamNetworkingSockets *interface, std::string uuid, HSteamNetConnection hConnection, DiscordAuth *pDiscordAuth );
    ~Client();

    void Authenticate();

#pragma region Getters + Setters
    std::string GetUuid();

    Player *GetPlayer();
    void SetPlayer( Player *pPlayer );

    void SetAuthenticated( bool newState );

#pragma endregion

   private:
    bool SendMessage( Packet packet, uint32 size, int nSendFlags );
    bool authenticated = false;
};
