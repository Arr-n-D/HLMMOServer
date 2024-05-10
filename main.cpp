#include "networking/NetworkManager.hpp"

using namespace Networking;

int main() {
    NetworkManager networkManager;
    networkManager.Init();
    networkManager.StartServer();
    return 0;
}

