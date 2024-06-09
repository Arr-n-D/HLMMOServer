#include "Networking/NetworkManager.hpp"

#include <thread>

#include "Networking/network_types.hpp"
#include "spdlog/spdlog.h"
using json = nlohmann::json;

using namespace Networking;

NetworkManager *NetworkManager::s_pCallbackInstance = nullptr;
bool NetworkManager::g_bQuit = false;
SteamNetworkingMicroseconds NetworkManager::g_logTimeZero;

NetworkManager::NetworkManager( DiscordAuth *pDiscordAuth ) {
    this->m_pDiscordAuth = pDiscordAuth;
}

NetworkManager::~NetworkManager() {
}

void NetworkManager::Init() {
    SteamDatagramErrMsg errMsg;
    if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
        FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );

    g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

    SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Everything, DebugOutput );

    this->InitializeAuthServer();
}

void NetworkManager::StartServer() {
    // Start listening
    m_pInterface = SteamNetworkingSockets();

    SteamNetworkingIPAddr serverLocalAddr;
    serverLocalAddr.Clear();
    serverLocalAddr.m_port = 27015;
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)SteamNetConnectionStatusChangedCallback );
    m_hListenSock = m_pInterface->CreateListenSocketIP( serverLocalAddr, 1, &opt );

    if ( m_hListenSock == k_HSteamListenSocket_Invalid )
        FatalError( "Failed to listen on port %d", 27015 );
    m_hPollGroup = m_pInterface->CreatePollGroup();
    if ( m_hPollGroup == k_HSteamNetPollGroup_Invalid )
        FatalError( "Failed to listen on port %d", 27015 );

    spdlog::info( "Listening on port {0}", 27015 );

    while ( !NetworkManager::g_bQuit ) {
        // PollIncomingMessages();
        PollConnectionStateChanges();
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
}

void NetworkManager::InitializeAuthServer() {
    spdlog::info( "Initializing Auth Server" );

    struct sockaddr_un server_addr;
    int server_socket;
    char *socket_path = "/tmp/icp-test";

    memset( &server_addr, 0, sizeof( server_addr ) );

    if ( ( this->m_nAuthServerSocket = socket( AF_UNIX, SOCK_STREAM, 0 ) ) == -1 ) {
        spdlog::error( "Auth server socket failed to initialize, exiting" );
        exit( 1 );
    }

    server_addr.sun_family = AF_UNIX;
    strcpy( server_addr.sun_path, socket_path );
    int len = sizeof( server_addr );

    // unlink the file before bind, unless it can't bind
    unlink( socket_path );

    if ( bind( this->m_nAuthServerSocket, (struct sockaddr *)&server_addr, len ) == -1 ) {
        spdlog::error( "Auth server socket failed to bind, exiting" );
        close( server_socket );
        exit( 1 );
    }

    if ( listen( this->m_nAuthServerSocket, 1 ) == -1 ) {
        spdlog::error( "Auth server socket failed to listen, exiting" );
        close( this->m_nAuthServerSocket );
        exit( 1 );
    }

    this->AcceptAuthServerConnection();

    // create a thread thread to poll incoming messages
    std::thread authServerPollThread( &NetworkManager::PollIncomingAuthMessages, this );
    authServerPollThread.detach();
}

void NetworkManager::AcceptAuthServerConnection() {
    this->m_nAuthServerSocketClient = accept( this->m_nAuthServerSocket, NULL, NULL );
    if ( this->m_nAuthServerSocketClient == -1 ) {
        spdlog::error( "Auth server socket failed to accept, exiting" );
        close( this->m_nAuthServerSocket );
        exit( 1 );
    }

    spdlog::info( "Auth server initialized" );
}

void NetworkManager::PollIncomingAuthMessages() {
    spdlog::info( "Polling incoming messages from auth server" );
    char buffer[1024];
    memset( buffer, 0, sizeof( buffer ) );

    while ( !NetworkManager::g_bQuit && this->m_nAuthServerSocketClient ) {
        int receivedBytes = recv( this->m_nAuthServerSocketClient, buffer, sizeof( buffer ), 0 );

        if ( receivedBytes == -1 ) {
            spdlog::error( "Error receiving message from auth server: {}", strerror( errno ) );
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

            continue;
        }

        if ( receivedBytes == 0 ) {
            this->AcceptAuthServerConnection();
            spdlog::warn( "Connection closed by auth server, retrying" );
            break;
        }

        // @TODO #11 - Find out why this is happening we get a weird Unicode Character 'FORM FEED (FF)' (U+000C) at the end of the buffer
        buffer[receivedBytes - 1] = '\0';
        json p = json::parse( buffer );
        LoginMessage message = p["data"].get<LoginMessage>();

        Client *found_client = this->GetClientByUuid( message.state );

        if ( found_client == nullptr ) {
            spdlog::warn( "Client not found with state {}", message.state );
            std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );

            continue;
        }

        found_client->SetAuthenticated( true );
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    close( this->m_nAuthServerSocketClient );  // Clean up socket
}

void NetworkManager::OnClientConnecting( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
    // assert( m_mapClients.find( pInfo->m_hConn ) == m_mapClients.end() );
    // assert (m_vecClients.find( pInfo->m_hConn ) == m_vecClients.end());
    // find client with pInfo->m_hConn in m_vecClients

    Client *existingClient = this->GetClientByConnectionHandle( pInfo->m_hConn );

    if ( existingClient != nullptr ) {
        spdlog::warn( "Client already exists with connection {}", pInfo->m_hConn );

        return;
    }

    spdlog::info( "Connection request from {}", pInfo->m_info.m_szConnectionDescription );

    if ( m_pInterface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK ) {
        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
        spdlog::warn( "Can't accept connection.  (It was already closed?)" );  // Add more logging

        return;
    }

    // Assign the poll group
    if ( !m_pInterface->SetConnectionPollGroup( pInfo->m_hConn, m_hPollGroup ) ) {
        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
        spdlog::warn( "Failed to set poll group" );

        return;
    }

    OnClientConnected( pInfo );
}

void NetworkManager::OnClientConnected( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
    spdlog::info( "Client connected: {}", pInfo->m_info.m_szConnectionDescription );
    std::string uuidStr = boost::uuids::to_string( boost::uuids::random_generator()() );
    Client *client = new Client( this->m_pInterface, uuidStr, pInfo->m_hConn, this->m_pDiscordAuth );

    client->Authenticate();
    m_vecClients.push_back( client );
}

void NetworkManager::OnClientDisconnect( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
    switch ( pInfo->m_info.m_eState ) {
        // A disruption in the connection has been detected locally. (E.g. timeout,
        // local internet connection disrupted, etc.)
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected ) {
                // Locate the client.  Note that it should have been found, because this
                // is the only codepath where we remove clients (except on shutdown),
                // and connection change callbacks are dispatched in queue order.
                // client->getstate

                Client *client = this->GetClientByConnectionHandle( pInfo->m_hConn );

                std::vector<Client *>::iterator foundClient = this->m_vecClients.begin();

                for ( ; foundClient != this->m_vecClients.end(); foundClient++ ) {
                    if ( ( *foundClient.base() )->GetConnection() != pInfo->m_hConn ) {
                        continue;
                    }

                    break;
                }

                if ( foundClient == this->m_vecClients.end() ) {
                    spdlog::warn( "Could not find client with handle {}", pInfo->m_hConn );

                    return;
                }

                // Select appropriate log messages
                const char *pszDebugLogAction;
                if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally ) {
                    pszDebugLogAction = "problem detected locally";
                    // fill message to send to other clients
                } else {
                    // Note that here we could check the reason code to see if
                    // it was a "usual" connection or an "unusual" one.
                    pszDebugLogAction = "closed by peer";
                    // fill message to send to other clients
                }

                // Spew something to our own log.  Note that because we put their nick
                // as the connection description, it will show up, along with their
                // transport-specific data (e.g. their IP address)
                spdlog::debug( "Connection {} {}, reason {}: {}",
                               pInfo->m_info.m_szConnectionDescription,
                               pszDebugLogAction,
                               pInfo->m_info.m_eEndReason,
                               pInfo->m_info.m_szEndDebug );

                this->m_vecClients.erase( foundClient );

                // Send a message so everybody else knows what happened
                // SendStringToAllClients(temp);
            } else {
                assert( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting );
            }

            m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
            break;
        }
            /* code */

        default:
            break;
    }
}

void NetworkManager::OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
    char temp[1024];

    // What's the state of the connection?
    switch ( pInfo->m_info.m_eState ) {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            OnClientDisconnect( pInfo );
        }

        case k_ESteamNetworkingConnectionState_Connecting: {
            OnClientConnecting( pInfo );
            // This must be a new connection
        }

        default:
            // Silences -Wswitch
            break;
    }
}

void NetworkManager::PollConnectionStateChanges() {
    s_pCallbackInstance = this;
    m_pInterface->RunCallbacks();
}

bool NetworkManager::SendMessageToPlayer( HSteamNetConnection hConn, Packet packet, uint32 size, int nSendFlags ) {
    for ( auto &client : this->m_vecClients ) {
        if ( client->GetConnection() != hConn ) {
            continue;
        }

        client->SendMessage( packet, size, nSendFlags );
    }
}

bool NetworkManager::SendMessageToAllPlayers( Packet packet, uint32 size, int nSendFlags ) {
    for ( auto &client : this->m_vecClients ) {
        client->SendMessage( packet, size, nSendFlags );
    }
}

Client *Networking::NetworkManager::GetClientByUuid( std::string uuidToFind ) {
    Client *foundClient = nullptr;

    for ( auto &client : this->m_vecClients ) {
        if ( client->GetUuid() != uuidToFind ) {
            continue;
        }

        foundClient = client;
        break;
    }

    return foundClient;
}

Client *Networking::NetworkManager::GetClientByConnectionHandle( HSteamNetConnection connectionHandle ) {
    Client *foundClient = nullptr;

    for ( auto &client : this->m_vecClients ) {
        if ( client->GetConnection() != connectionHandle ) {
            continue;
        }

        foundClient = client;
        break;
    }

    return foundClient;
}
