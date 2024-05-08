#pragma once

#include <steam/isteamnetworkingutils.h>
class Player;

class Client {
   public:
    Client();
    ~Client();

    void SetPlayer(Player *pPlayer) { m_pPlayer = pPlayer; }
    Player *GetPlayer() { return m_pPlayer; }

    private:
        HSteamNetConnection m_hConnection;
        Player *m_pPlayer;
};