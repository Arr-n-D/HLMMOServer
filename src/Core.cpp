#include <Core.hpp>
#include <iostream>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;



Core::Core(){};

Core::~Core(){};

void Core::Initialize() {
    std::cout << "Initializing Core" << std::endl;

    // std::string clientId = getenv( "DISCORD_CLIENT_ID" );
    // std::string secret = getenv( "DISCORD_SECRET" );
    // std::string redirect_uri = getenv( "DISCORD_REDIRECT_URI" );

    char const* clientId = getenv( "DISCORD_CLIENT_ID" );
    char const* discordSecret = getenv( "DISCORD_SECRET" );
    char const* redirectURI = getenv( "DISCORD_REDIRECT_URI" );
    if ( clientId == NULL || discordSecret == NULL || redirectURI == NULL ) {
        // exit if the environment variables are not set
        std::cout << "Missing environment variables" << std::endl;
       std::this_thread::sleep_for(5000ms);
        exit( 1 );
    } else {
        std::string sClientId( clientId );
        std::string sSecret( discordSecret );
        std::string RedirectURI( redirectURI );

        std::cout << "Client ID: " << sClientId << std::endl;
        std::cout << "Secret: " << sSecret << std::endl;
        std::cout << "Redirect URI: " << RedirectURI << std::endl;

        m_pDiscordAuth = new DiscordAuth( sClientId, sSecret, RedirectURI );
        m_pNetworkManager = new Networking::NetworkManager( m_pDiscordAuth );
        m_pNetworkManager->Init();
        m_pNetworkManager->StartServer();
        //  ...
    }
}