#include <Core.hpp>
#include <thread>

#include "spdlog/spdlog.h"

using namespace std::chrono_literals;

Core::Core(){};

Core::~Core(){};

void Core::Initialize() {
    spdlog::info( "Initializing Core" );

    std::string sClientId = GetEnvironmentVariable( "DISCORD_CLIENT_ID" );
    std::string sSecret = GetEnvironmentVariable( "DISCORD_SECRET" );
    std::string RedirectURI = GetEnvironmentVariable( "DISCORD_REDIRECT_URI" );

    spdlog::debug( "Client ID: {}", sClientId );
    spdlog::debug( "Client Secret: {}", sSecret );
    spdlog::debug( "Redirect URI: {}", RedirectURI );

    m_pDiscordAuth = new DiscordAuth( sClientId, sSecret, RedirectURI );
    m_pNetworkManager = new Networking::NetworkManager( m_pDiscordAuth );
    m_pNetworkManager->Init();
    m_pNetworkManager->StartServer();
}

std::string Core::GetEnvironmentVariable( std::string environmentVariableName ) {
    const char* outValue = getenv( environmentVariableName.data() );

    if ( outValue != NULL ) {
        return std::string{ outValue };
    }

    spdlog::critical( "Missing environment variable: {}", environmentVariableName );
    exit( 1 );
}
