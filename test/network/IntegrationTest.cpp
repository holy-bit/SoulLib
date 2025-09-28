#include "network/client/Client.h"
#include <gtest/gtest.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include "debug/Debug.h"

// Renamed local function to avoid conflict with system htonll
static uint64_t hostToNetwork64(uint64_t val) {
    // Split into high and low 32-bit words
    uint32_t high32 = static_cast<uint32_t>(val >> 32);
    uint32_t low32  = static_cast<uint32_t>(val & 0xFFFFFFFF);
    uint32_t highN = htonl(high32);
    uint32_t lowN  = htonl(low32);
    // Combine high and low in network byte order: highN as high word
    return (static_cast<uint64_t>(highN) << 32) | lowN;
}

// Helper to convert Windows socket errors to string
std::string getWinsockErrorString(int error) {
    std::ostringstream oss;
    oss << "Error " << error << ": ";
    
    switch (error) {
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

class ClientServerFixture : public ::testing::Test {
protected:
    std::thread serverThread;
    uint16_t port = 55555; // Changed port to avoid conflicts
    uint64_t sendValue = 123456789ULL;

    void SetUp() override {
        // Initialize Winsock for server
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);
        
        std::cout << "=== Setting up test server on port " << port << " ===" << std::endl;
        
        // Start server thread
        serverThread = std::thread([&]() {
            SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (listenSock == INVALID_SOCKET) {
                std::cerr << "Server socket creation failed: " << getWinsockErrorString(WSAGetLastError()) << std::endl;
                return;
            }
            
            // Set socket reuse to avoid "Address already in use" errors
            int reuse = 1;
            if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
                std::cerr << "Failed to set SO_REUSEADDR: " << getWinsockErrorString(WSAGetLastError()) << std::endl;
            }
            
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY; // Use any available interface
            addr.sin_port = htons(port);
            
            if (bind(listenSock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
                int error = WSAGetLastError();
                std::cerr << "Server bind failed: " << getWinsockErrorString(error) << std::endl;
                closesocket(listenSock);
                return;
            }
            
            std::cout << "Server socket bound successfully" << std::endl;
            
            if (listen(listenSock, 1) == SOCKET_ERROR) {
                int error = WSAGetLastError();
                std::cerr << "Server listen failed: " << getWinsockErrorString(error) << std::endl;
                closesocket(listenSock);
                return;
            }
            
            std::cout << "Server listening for connections..." << std::endl;
            
            // Set timeout for accept
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(listenSock, &readSet);
            
            // Wait up to 3 seconds for a client connection
            timeval timeout;
            timeout.tv_sec = 3;
            timeout.tv_usec = 0;
            
            int selectResult = select(0, &readSet, nullptr, nullptr, &timeout);
            if (selectResult <= 0) {
                if (selectResult == 0) {
                    std::cerr << "Server accept timeout" << std::endl;
                } else {
                    std::cerr << "Server select failed: " << getWinsockErrorString(WSAGetLastError()) << std::endl;
                }
                closesocket(listenSock);
                return;
            }
            
            std::cout << "Client connection pending, accepting..." << std::endl;
            
            SOCKET clientSock = accept(listenSock, nullptr, nullptr);
            if (clientSock == INVALID_SOCKET) {
                int error = WSAGetLastError();
                std::cerr << "Server accept failed: " << getWinsockErrorString(error) << std::endl;
                closesocket(listenSock);
                return;
            }
            
            std::cout << "Client connected, waiting for request..." << std::endl;
            
            // Set receive timeout for client socket
            timeout.tv_sec = 2;
            if (setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
                std::cerr << "Failed to set client socket timeout: " << getWinsockErrorString(WSAGetLastError()) << std::endl;
            }
            
            // Wait for client request
            char requestBuffer[128];
            int received = recv(clientSock, requestBuffer, sizeof(requestBuffer) - 1, 0);
            
            if (received > 0) {
                requestBuffer[received] = '\0';  // Null terminate
                std::cout << "Server received request: " << requestBuffer << std::endl;
                
                // Client has sent the GET_TIME request, now send the timestamp
                uint64_t netTs = hostToNetwork64(sendValue);
                int bytesSent = send(clientSock, reinterpret_cast<const char*>(&netTs), sizeof(netTs), 0);
                
                if (bytesSent == sizeof(netTs)) {
                    std::cout << "Server sent timestamp: " << sendValue << " (" << bytesSent << " bytes)" << std::endl;
                } else {
                    std::cerr << "Server failed to send complete timestamp: sent " << bytesSent << " of " 
                              << sizeof(netTs) << " bytes" << std::endl;
                }
            } else {
                if (received == 0) {
                    std::cerr << "Client disconnected before sending request" << std::endl;
                } else {
                    std::cerr << "Error receiving client request: " << getWinsockErrorString(WSAGetLastError()) << std::endl;
                }
            }
            
            // Keep connection open for a moment to ensure client can read the response
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            
            std::cout << "Server closing connection" << std::endl;
            shutdown(clientSock, SD_SEND);
            closesocket(clientSock);
            closesocket(listenSock);
        });
        
        // Give server time to start and bind to address
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    void TearDown() override {
        std::cout << "=== Tearing down test server ===" << std::endl;
        if (serverThread.joinable()) {
            serverThread.join();
        }
        WSACleanup();
    }
};

TEST_F(ClientServerFixture, RequestServerTimeReturnsKnownValue) {
    std::cout << "Starting client test..." << std::endl;
    Client client("127.0.0.1", port);
    
    std::cout << "Calling requestServerTime()..." << std::endl;
    std::string result = client.requestServerTime();
    
    std::cout << "Client received result: " << (result.empty() ? "EMPTY" : result) << std::endl;
    
    ASSERT_FALSE(result.empty()) << "Result was empty, server time request failed";
    if (!result.empty()) {
        uint64_t received = std::stoull(result);
        EXPECT_EQ(received, sendValue) << "Expected: " << sendValue << ", got: " << received;
    }
}

TEST_F(ClientServerFixture, ClientIsConnectedAfterRequest) {
    Client client("127.0.0.1", port);
    client.requestServerTime();
    EXPECT_TRUE(client.isConnected());
}