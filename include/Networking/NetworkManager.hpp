#pragma once
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <string.h>

#include <chrono>
#include <map>
#include <string>
#include <thread>

#include "Client.hpp"

class DiscordAuth;

namespace Networking {
class NetworkManager {
   public:
    NetworkManager(DiscordAuth *pDiscordAuth);
    ~NetworkManager();

    void Init();
    void StartServer();

    std::map<HSteamNetConnection, Client *> m_mapClients;

#pragma region StaticRegion
    static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg ) {
        SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
        printf( "%10.6f %s\n", time * 1e-6, pszMsg );
        fflush( stdout );
        if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug ) {
            fflush( stdout );
            fflush( stderr );
        }
    }

    static void FatalError( const char *fmt, ... ) {
        char text[2048];
        va_list ap;
        va_start( ap, fmt );
        vsprintf( text, fmt, ap );
        va_end( ap );
        char *nl = strchr( text, '\0' ) - 1;
        if ( nl >= text && *nl == '\n' )
            *nl = '\0';
        DebugOutput( k_ESteamNetworkingSocketsDebugOutputType_Everything, text );
    }

    static void SteamNetConnectionStatusChangedCallback( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
        s_pCallbackInstance->OnSteamNetConnectionStatusChanged( pInfo );
    }
#pragma endregion

   private:
    HSteamListenSocket m_hListenSock;
    HSteamNetPollGroup m_hPollGroup;
    ISteamNetworkingSockets *m_pInterface;
    DiscordAuth *m_pDiscordAuth;
    static NetworkManager *s_pCallbackInstance;  // Used for OnSteamNetConnectionStatusChanged since we cannot cast a member function to a void pointer
    static bool g_bQuit;
    static SteamNetworkingMicroseconds g_logTimeZero;

    void PollIncomingMessages();


#pragma region Event Callbacks
    void OnClientConnecting( SteamNetConnectionStatusChangedCallback_t *pInfo );
    void OnClientConnected( SteamNetConnectionStatusChangedCallback_t *pInfo );
    void OnClientDisconnect( SteamNetConnectionStatusChangedCallback_t *pInfo );
    void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo );
    void OnMessagedReceived();
#pragma endregion

    void PollConnectionStateChanges();
};
}  // namespace Networking
