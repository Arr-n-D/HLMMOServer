#include "DiscordAuth.hpp"

DiscordAuth::DiscordAuth(std::string clientId, std::string secret, std::string redirect_uri) {
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
