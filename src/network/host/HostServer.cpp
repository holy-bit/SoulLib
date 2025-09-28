#include "network/host/HostServer.h"
#include "debug/Debug.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iomanip> // For std::put_time
#include <sstream> // For std::ostringstream
#include <atomic>
#pragma comment(lib, "Ws2_32.lib")

HostServer::HostServer() {
    // Constructor logic if needed
}

void HostServer::start() {
    Debug::Debug("Time Server is starting...", {__FILE__, __LINE__, __FUNCTION__});

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        Debug::Error("WSAStartup failed.", {__FILE__, __LINE__, __FUNCTION__});
        return;
    }

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        Debug::Error("Socket creation failed.", {__FILE__, __LINE__, __FUNCTION__});
        WSACleanup();
        return;
    }

    // Bind the socket
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        Debug::Error("Bind failed.", {__FILE__, __LINE__, __FUNCTION__});
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Listen for incoming connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        Debug::Error("Listen failed.", {__FILE__, __LINE__, __FUNCTION__});
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    Debug::Debug("Server is listening on port 8080...", {__FILE__, __LINE__, __FUNCTION__});

    // Atomic flag to control the time printing thread
    std::atomic<bool> running(true);

    // Start a thread to print the current server time every 5 seconds
    std::thread timePrinter([&running]() {
        while (running) {
            auto now = std::chrono::system_clock::now();
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
            Debug::Debug("Current server time: " + oss.str(), {__FILE__, __LINE__, __FUNCTION__});
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    });

    // Accept multiple client connections in a loop
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            Debug::Error("Accept failed.", {__FILE__, __LINE__, __FUNCTION__});
            continue; // Continue to accept new connections
        }

        Debug::Debug("Client connected.", {__FILE__, __LINE__, __FUNCTION__});

        // Handle the client in a separate thread
        std::thread([this, clientSocket]() {
            uint64_t serverTime = timeService.getCurrentTime();
            send(clientSocket, reinterpret_cast<const char*>(&serverTime), sizeof(serverTime), 0);
            handleClientDisconnection(clientSocket);
        }).detach();
    }

    // Clean up
    running = false;
    timePrinter.join();
    closesocket(serverSocket);
    WSACleanup();
}

void HostServer::synchronizeServerTime() {
    auto start = std::chrono::steady_clock::now();
    uint64_t serverTime = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
    timeService.syncWithServerTime(serverTime);
    Debug::Debug("Server time synchronized to: " + std::to_string(serverTime) + " ms", {__FILE__, __LINE__, __FUNCTION__});
}

void HostServer::runServerLoop() {
    while (true) {
        timeService.updateRealTime();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        Debug::Debug("Current server time: " + std::to_string(timeService.getCurrentTime()) + " ms", {__FILE__, __LINE__, __FUNCTION__});

        // Check for client disconnection
        char buffer[1];
        int result = recv(clientSocket, buffer, sizeof(buffer), MSG_PEEK);
        if (result == 0 || result == SOCKET_ERROR) {
            handleClientDisconnection(clientSocket);
            break;
        }
    }
}

void HostServer::handleClientDisconnection(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // Null-terminate the received data
        std::string message(buffer);
        if (message == "LOGOUT") {
            Debug::Debug("Client sent logout message.", {__FILE__, __LINE__, __FUNCTION__});
        }
    }

    Debug::Debug("Client disconnected.", {__FILE__, __LINE__, __FUNCTION__});
    closesocket(clientSocket);
}