#include "Networking/NetworkManager.hpp"

#include "Networking/network_types.hpp"
#include "spdlog/spdlog.h"

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
    printf( "Initializing Auth Server\n" );
    char *socket_path = "/tmp/icp-test";
    struct sockaddr_un addr;
    int fileDescriptor;

    if ( ( fileDescriptor = socket( AF_UNIX, SOCK_STREAM, 0 ) ) == -1 ) {
        printf( "Auth server socket failed to initialize, exiting" );
        exit( 1 );
    }

    printf( "Auth server socket initialized\n" );

    memset( &addr, 0, sizeof( addr ) );
    addr.sun_family = AF_UNIX;
    if ( *socket_path == '\0' ) {
        *addr.sun_path = '\0';
        strncpy( addr.sun_path + 1, socket_path + 1, sizeof( addr.sun_path ) - 2 );
    } else {
        strncpy( addr.sun_path, socket_path, sizeof( addr.sun_path ) - 1 );
        unlink( socket_path );
    }

    printf( "Auth server socket path set\n" );

    if ( bind( fileDescriptor, (struct sockaddr *)&addr, sizeof( addr ) ) == -1 ) {
        perror( "bind error" );
        exit( -1 );
    }

    printf( "Auth server socket bound\n" );

    if ( listen( fileDescriptor, 1 ) == -1 ) {
        printf( "Auth server socket failed to listen, exiting" );
        exit( 1 );
    }

    printf( "Auth server socket listening\n" );

    while ( 1 ) {
        if ( ( this->m_nAuthServerSocketClient = accept( fileDescriptor, NULL, NULL ) ) == -1 ) {
            perror( "accept error" );
            continue;
        }
    }
}

void NetworkManager::OnClientConnecting( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
    assert( m_mapClients.find( pInfo->m_hConn ) == m_mapClients.end() );

    spdlog::debug( "Connection request from {}", pInfo->m_info.m_szConnectionDescription );

    if ( m_pInterface->AcceptConnection( pInfo->m_hConn ) != k_EResultOK ) {
        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
        spdlog::warn( "Can't accept connection.  (It was already closed?)" );  // Add more logging
    }

    // Assign the poll group
    if ( !m_pInterface->SetConnectionPollGroup( pInfo->m_hConn, m_hPollGroup ) ) {
        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
        spdlog::warn( "Failed to set poll group" );
    }

    OnClientConnected( pInfo );
}

void NetworkManager::OnClientConnected( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
    Client *client = new Client( this->m_pInterface, boost::uuids::random_generator()(), pInfo->m_hConn, this->m_pDiscordAuth );
    if ( client->Authenticate() ) {
        m_mapClients[pInfo->m_hConn] = client;
    } else {
        m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
    }
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
                auto itClient = m_mapClients.find( pInfo->m_hConn );
                assert( itClient != m_mapClients.end() );

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

                m_mapClients.erase( itClient );

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
