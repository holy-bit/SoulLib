#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

class Client {
public:
    Client(const std::string& serverAddress, int port);
    bool connectToServer();
    void updateServerTime();
    bool isConnected() const;
    void disconnectFromServer();
    std::string requestServerTime();

private:
    std::string serverAddress;
    int port;
    bool connected;
    SOCKET clientSocket;
    void simulateConnection();
};

#endif // CLIENT_H