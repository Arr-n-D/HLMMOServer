#include "Client.hpp"

Client::Client( boost::uuids::uuid uuid, HSteamNetConnection hConnection ) {
    this->uuid = uuid;
    this->m_hConnection = hConnection;
}

Client::~Client() {
}
