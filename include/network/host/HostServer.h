#ifndef HOST_SERVER_H
#define HOST_SERVER_H

#include "time/TimeService.h"
#include "time/TimerManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

class HostServer {
public:
    HostServer();
    void start();

private:
    SOCKET clientSocket;
    TimeService timeService;
    TimerManager timerManager;
    void synchronizeServerTime();
    void runServerLoop();
    void handleClientDisconnection(SOCKET clientSocket);


};

#endif // HOST_SERVER_H