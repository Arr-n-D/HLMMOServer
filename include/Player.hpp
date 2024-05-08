#pragma once

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include "Client.hpp"

class Player {
   public:
    Player();
    ~Player();

    private:
        Client *m_pClient;
};