#include "DiscordAuth.hpp"

DiscordAuth::DiscordAuth( std::string clientId, std::string secret, std::string redirect_uri ) {
    this->clientId = clientId;
    this->secret = secret;
    this->redirect_uri = redirect_uri;
}

DiscordAuth::~DiscordAuth() {
}

std::string DiscordAuth::GetClientId() {
    return this->clientId;
}

std::string DiscordAuth::GetSecret() {
    return this->secret;
}

std::string DiscordAuth::GetRedirectUri() {
    return this->redirect_uri;
}

std::string DiscordAuth::BuildAuthUrlForClient( Client *client ) {
    std::string url = std::format( "https://discord.com/oauth2/authorize?client_id={0}&response_type=code&redirect_uri={1}&scope=identify+email&state={2}", this->clientId, this->redirect_uri, client->GetUuid() );
    return url;
}
