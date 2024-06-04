#pragma once

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include "Networking/network_types.hpp"

class Player;
class DiscordAuth;

class Client {
   public:
    Client(ISteamNetworkingSockets *interface, boost::uuids::uuid uuid, HSteamNetConnection hConnection, DiscordAuth *pDiscordAuth);
    ~Client();

    void SetPlayer(Player *pPlayer) { m_pPlayer = pPlayer; }
    Player *GetPlayer() { return m_pPlayer; }
    boost::uuids::uuid GetUuid() { return uuid; }
    bool Authenticate();

    private:
        HSteamNetConnection m_hConnection;
        Player *m_pPlayer;
        boost::uuids::uuid uuid;
        DiscordAuth *m_pDiscordAuth;
        ISteamNetworkingSockets *m_pInterface;
        bool SendMessage(Packet packet, uint32 size, int nSendFlags);
        
};