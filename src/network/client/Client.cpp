#include "network/client/Client.h"
#include "debug/Debug.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctime>
#include <sstream>
#pragma comment(lib, "Ws2_32.lib")

// Helper function to get readable error messages from Winsock errors
std::string getSocketErrorString(int errorCode) {
    std::ostringstream oss;
    oss << "Error " << errorCode << ": ";
    
    switch (errorCode) {
        case WSAEACCES: oss << "Permission denied"; break;
        case WSAEADDRINUSE: oss << "Address already in use"; break;
        case WSAEADDRNOTAVAIL: oss << "Cannot assign requested address"; break;
        case WSAECONNREFUSED: oss << "Connection refused"; break;
        case WSAETIMEDOUT: oss << "Connection timed out"; break;
        case WSAENETUNREACH: oss << "Network is unreachable"; break;
        case WSAENOTCONN: oss << "Socket is not connected"; break;
        default: oss << "Unknown error"; break;
    }
    
    return oss.str();
}

Client::Client(const std::string& serverAddress, int port)
    : serverAddress(serverAddress), port(port), connected(false) {}

bool Client::connectToServer() {
    DEBUG_LOG("Connecting to server at " + serverAddress + ":" + std::to_string(port) + "...");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        int error = WSAGetLastError();
        DEBUG_ERROR("WSAStartup failed: " + getSocketErrorString(error));
        return false;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        DEBUG_ERROR("Socket creation failed: " + getSocketErrorString(error));
        WSACleanup();
        return false;
    }

    // Set receive timeout to 3 seconds to avoid long hangs
    int timeout = 3000; // 3 seconds in milliseconds
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        DEBUG_WARNING("Failed to set socket receive timeout: " + getSocketErrorString(error));
        // Continue anyway, not critical
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(static_cast<u_short>(port));

    DEBUG_LOG("Attempting connection to " + serverAddress + ":" + std::to_string(port));
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        DEBUG_ERROR("Connection to server failed: " + getSocketErrorString(error));
        closesocket(clientSocket);
        WSACleanup();
        return false;
    }

    DEBUG_LOG("Successfully connected to the server");
    connected = true;
    return true;
}

void Client::updateServerTime() {
    if (!connected) {
        DEBUG_WARNING("Not connected to the server. Cannot update time.");
        return;
    }

    DEBUG_LOG("Requesting server time update...");

    // Send a request to the server for the current time
    const std::string requestMessage = "GET_TIME";
    if (send(clientSocket, requestMessage.c_str(), requestMessage.size(), 0) == SOCKET_ERROR) {
        DEBUG_ERROR("Failed to send time update request to the server.");
        return;
    }

    // Receive the server's response
    char buffer[256] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        DEBUG_ERROR("Failed to receive server time or connection closed.");
        return;
    }

    buffer[bytesReceived] = '\0'; // Null-terminate the received data
    DEBUG_LOG("Received server time: " + std::string(buffer));

    DEBUG_LOG("Server time updated successfully: " + std::string(buffer));
}

void Client::simulateConnection() {
    // Simulate a connection process
    std::this_thread::sleep_for(std::chrono::seconds(2));
    connected = true; // Simulate a successful connection
}

bool Client::isConnected() const {
    return connected;
}

void Client::disconnectFromServer() {
    if (connected) {
        DEBUG_LOG("Disconnecting from server...");

        // Send logout message to the server
        const std::string logoutMessage = "LOGOUT";
        send(clientSocket, logoutMessage.c_str(), logoutMessage.size(), 0);

        connected = false;
        closesocket(clientSocket);
        WSACleanup();
    } else {
        DEBUG_WARNING("Already disconnected from the server.");
    }
}

std::string Client::requestServerTime() {
    if (!isConnected()) {
        DEBUG_LOG("Not connected, attempting to connect to server...");
        if (!connectToServer()) {
            DEBUG_ERROR("Failed to connect to server for time request");
            return "";
        }
    }

    // Send a dummy request to the server first to indicate we're ready for data
    DEBUG_LOG("Sending GET_TIME request to server");
    const std::string requestMessage = "GET_TIME";
    if (send(clientSocket, requestMessage.c_str(), requestMessage.size(), 0) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        DEBUG_ERROR("Failed to send request to the server: " + getSocketErrorString(error));
        return "";
    }
    
    DEBUG_LOG("Request sent, waiting for server response");
    
    // Small delay to ensure server processes the request
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    uint64_t netTime = 0;
    char* dataPtr = reinterpret_cast<char*>(&netTime);
    size_t totalReceived = 0;
    const size_t toReceive = sizeof(netTime);
    
    DEBUG_LOG("Starting to receive " + std::to_string(toReceive) + " bytes of timestamp data");
    
    // Loop until full 8-byte timestamp is received
    while (totalReceived < toReceive) {
        DEBUG_LOG("Waiting to receive " + std::to_string(toReceive - totalReceived) + " more bytes...");
        
        int bytes = recv(clientSocket, dataPtr + totalReceived, static_cast<int>(toReceive - totalReceived), 0);
        if (bytes == SOCKET_ERROR) {
            int errorCode = WSAGetLastError();
            DEBUG_ERROR("Socket error receiving timestamp: " + getSocketErrorString(errorCode));
            return "";
        } 
        else if (bytes == 0) {
            // Connection closed by server
            if (totalReceived == 0) {
                DEBUG_ERROR("Connection closed by server before receiving timestamp");
                return "";
            }
            DEBUG_ERROR("Connection closed by server, but received " + std::to_string(totalReceived) + " of " + 
                        std::to_string(toReceive) + " bytes");
            break;
        }
        
        totalReceived += static_cast<size_t>(bytes);
        DEBUG_LOG("Received " + std::to_string(bytes) + " bytes, total: " + std::to_string(totalReceived) + "/" + std::to_string(toReceive));
    }
    
    if (totalReceived < toReceive) {
        DEBUG_ERROR("Failed to receive full timestamp, got only " + std::to_string(totalReceived) + " of " + 
                    std::to_string(toReceive) + " bytes");
        return "";
    }

    DEBUG_LOG("Successfully received complete timestamp");

    // Network-to-host conversion (32-bit swaps)
    uint32_t highN = static_cast<uint32_t>(netTime >> 32);
    uint32_t lowN  = static_cast<uint32_t>(netTime & 0xFFFFFFFF);
    uint32_t high = ntohl(highN);
    uint32_t low  = ntohl(lowN);
    uint64_t serverMs = (static_cast<uint64_t>(high) << 32) | low;
    
    DEBUG_LOG("Converted timestamp value: " + std::to_string(serverMs));
    return std::to_string(serverMs);
}