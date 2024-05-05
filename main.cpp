#include "NetworkManager.hpp"

int main() {
    NetworkManager networkManager;
    networkManager.Init();
    networkManager.StartServer();
    return 0;
}