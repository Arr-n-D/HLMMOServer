#include "Client.hpp"

#include <DiscordAuth.hpp>
#include <Networking/network_types.hpp>

#include "DiscordAuth.hpp"
#include "spdlog/spdlog.h"

Client::Client( ISteamNetworkingSockets *m_pInterface, std::string uuid, HSteamNetConnection hConnection, DiscordAuth *pDiscordAuth ) {
    this->m_hConnection = hConnection;
    this->m_pPlayer = nullptr;
    this->uuid = uuid;
    this->m_pDiscordAuth = pDiscordAuth;
    this->m_pInterface = m_pInterface;
    this->authenticated = false;
}

Client::~Client() {
}

void Client::SetAuthenticated( bool newState ) {
    this->authenticated = newState;
}

std::string Client::GetUuid() {
    return this->uuid;
}

Player *Client::GetPlayer() {
    return this->m_pPlayer;
}

void Client::SetPlayer( Player *pPlayer ) {
    this->m_pPlayer = pPlayer;
}

HSteamNetConnection Client::GetConnection() {
    return this->m_hConnection;
}

void Client::Authenticate() {
    std::string url = this->m_pDiscordAuth->BuildAuthUrlForClient( this );

    GameMessageDiscordAuthRequest request = { url };

    // @TODO: #2 Export to a different method, clean code, nerd
    msgpack::sbuffer buffer;
    msgpack::pack( buffer, request );

    const char *data = buffer.data();

    Packet packet = {
        PacketType::GameMessage,
        0,
        buffer.size(),
        data };

    this->SendMessage( packet, buffer.size(), 0 );
}

bool Client::SendMessage( Packet packet, uint32 size, int nSendFlags ) {
    msgpack::sbuffer buffer2;
    msgpack::pack( buffer2, packet );

    this->m_pInterface->SendMessageToConnection( this->m_hConnection, buffer2.data(), (uint32)buffer2.size(), nSendFlags, nullptr );
}
