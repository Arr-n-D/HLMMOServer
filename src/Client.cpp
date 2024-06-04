#include "Client.hpp"
#include "DiscordAuth.hpp"

Client::Client( boost::uuids::uuid uuid, HSteamNetConnection hConnection, DiscordAuth *pDiscordAuth) {
    this->uuid = uuid;
    this->m_hConnection = hConnection;
}

Client::~Client() {
}

void Client::Authenticate() {
    std::string url = this->m_pDiscordAuth->BuildAuthUrlForClient(this);
    printf("Authenticating client with url: %s", url.c_str());
}
