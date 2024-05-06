#pragma once
#include <map>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <thread>
#include <chrono>
#include <assert.h>



class NetworkManager
{
public:
    NetworkManager();
    ~NetworkManager();

    void Init();
    void StartServer();

    std::map<HSteamNetConnection, std::string> m_mapClients;

    static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg)
    {
        SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
        printf("%10.6f %s\n", time * 1e-6, pszMsg);
        fflush(stdout);
        if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
        {
            fflush(stdout);
            fflush(stderr);
        }
    }

    static void FatalError(const char *fmt, ...)
    {
        char text[2048];
        va_list ap;
        va_start(ap, fmt);
        vsprintf_s(text, fmt, ap);
        va_end(ap);
        char *nl = strchr(text, '\0') - 1;
        if (nl >= text && *nl == '\n')
            *nl = '\0';
        DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Everything, text);
    }

    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo)
    {
        s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
    }

private:
    HSteamListenSocket m_hListenSock;
    HSteamNetPollGroup m_hPollGroup;
    ISteamNetworkingSockets *m_pInterface;
    static NetworkManager *s_pCallbackInstance; // Used for OnSteamNetConnectionStatusChanged since we cannot cast a member function to a void pointer
    static bool g_bQuit;
    static SteamNetworkingMicroseconds g_logTimeZero;

    void PollIncomingMessages();

    void OnEvent();
    void OnMessagedReceived();

    void OnClientConnect();
    void OnClientDisconnect();
    void PollConnectionStateChanges();
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo);
};

