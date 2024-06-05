#include "Client.hpp"

#include <DiscordAuth.hpp>
#include <Networking/network_types.hpp>

Client::Client( ISteamNetworkingSockets *m_pInterface, boost::uuids::uuid uuid, HSteamNetConnection hConnection, DiscordAuth *pDiscordAuth ) {
    this->m_pInterface = m_pInterface;
    this->uuid = uuid;
    this->m_hConnection = hConnection;
}

Client::~Client() {
}

bool Client::Authenticate() {
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
    
    return this->m_pInterface->SendMessageToConnection( this->m_hConnection,buffer2.data(), (uint32)buffer2.size(), nSendFlags, nullptr );
}