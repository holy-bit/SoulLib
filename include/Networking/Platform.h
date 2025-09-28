#pragma once

#if defined(_WIN32)
#   include <winsock2.h>
#   include <ws2tcpip.h>
#else
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <sys/socket.h>
#   include <sys/types.h>
#   include <unistd.h>
#endif

namespace soul::net::platform {
#if defined(_WIN32)
using SocketHandle = SOCKET;
inline constexpr SocketHandle invalid_socket = INVALID_SOCKET;
inline void close_socket(SocketHandle socket) { closesocket(socket); }
inline bool startup() {
    static bool initialized = false;
    if (!initialized) {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            return false;
        }
        initialized = true;
    }
    return true;
}
#else
using SocketHandle = int;
inline constexpr SocketHandle invalid_socket = -1;
inline void close_socket(SocketHandle socket) { ::close(socket); }
inline bool startup() { return true; }
#endif

} // namespace soul::net::platform
