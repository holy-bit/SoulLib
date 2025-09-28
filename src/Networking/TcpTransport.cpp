#include "Networking/TcpTransport.h"

#include <array>
#include <cstdint>
#include <optional>

#if !defined(_WIN32)
#   include <sys/select.h>
#endif

#include "Networking/Platform.h"

namespace soul::net {

namespace {

platform::SocketHandle create_tcp_socket() {
    if (!platform::startup()) {
        return platform::invalid_socket;
    }
    platform::SocketHandle handle{};
#if defined(_WIN32)
    handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    handle = ::socket(AF_INET, SOCK_STREAM, 0);
#endif
    return handle;
}

}

TcpTransport::TcpTransport(std::shared_ptr<soul::async::TaskScheduler> scheduler)
    : m_scheduler(std::move(scheduler)) {}

TcpTransport::~TcpTransport() {
    close();
}

bool TcpTransport::ensure_socket_created() {
    if (m_listenSocket != platform::invalid_socket) {
        return true;
    }
    m_listenSocket = create_tcp_socket();
    return m_listenSocket != platform::invalid_socket;
}

void TcpTransport::initialize_platform() {
#if defined(_WIN32)
    m_isWindows = true;
#endif
}

bool TcpTransport::bind(const Endpoint& endpoint) {
    initialize_platform();
    if (!ensure_socket_created()) {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = endpoint.port;
    addr.sin_addr.s_addr = endpoint.address;

    const auto result = ::bind(m_listenSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
#if defined(_WIN32)
    if (result == SOCKET_ERROR) {
        return false;
    }
    return ::listen(m_listenSocket, SOMAXCONN) != SOCKET_ERROR;
#else
    if (result < 0) {
        return false;
    }
    return ::listen(m_listenSocket, SOMAXCONN) == 0;
#endif
}

void TcpTransport::close() {
    if (m_listenSocket != platform::invalid_socket) {
        platform::close_socket(m_listenSocket);
        m_listenSocket = platform::invalid_socket;
    }
}

soul::async::Task<void> TcpTransport::send(const Endpoint& endpoint, Packet packet) {
    auto scheduler = m_scheduler;
    co_await scheduler->run_async([endpoint, packet = std::move(packet)]() mutable {
        platform::SocketHandle socket = create_tcp_socket();
        if (socket == platform::invalid_socket) {
            return;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = endpoint.port;
        addr.sin_addr.s_addr = endpoint.address;

        const auto result = ::connect(socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
#if defined(_WIN32)
        if (result == SOCKET_ERROR) {
            platform::close_socket(socket);
            return;
        }
#else
        if (result < 0) {
            platform::close_socket(socket);
            return;
        }
#endif

    auto headerBytes = encode_header(packet.header);
    std::uint32_t size = static_cast<std::uint32_t>(packet.payload.size());
    ::send(socket, reinterpret_cast<const char*>(&size), sizeof(size), 0);
    ::send(socket,
        reinterpret_cast<const char*>(headerBytes.data()),
        static_cast<int>(headerBytes.size()),
        0);
    if (size > 0) {
        ::send(socket,
            reinterpret_cast<const char*>(packet.payload.data()),
            static_cast<int>(packet.payload.size()),
            0);
    }
        platform::close_socket(socket);
    });
}

soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> TcpTransport::receive() {
    if (!ensure_socket_created()) {
        co_return std::nullopt;
    }

    auto scheduler = m_scheduler;
    co_return co_await scheduler->run_async([listenSocket = m_listenSocket]() -> std::optional<std::pair<Endpoint, Packet>> {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSocket, &readSet);

        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

#if defined(_WIN32)
        const int ready = ::select(0, &readSet, nullptr, nullptr, &timeout);
#else
        const int ready = ::select(static_cast<int>(listenSocket + 1), &readSet, nullptr, nullptr, &timeout);
#endif
        if (ready <= 0 || !FD_ISSET(listenSocket, &readSet)) {
            return std::nullopt;
        }

        sockaddr_in client{};
#if defined(_WIN32)
        int clientLen = static_cast<int>(sizeof(client));
#else
        socklen_t clientLen = sizeof(client);
#endif
        platform::SocketHandle socket = ::accept(listenSocket, reinterpret_cast<sockaddr*>(&client), &clientLen);
#if defined(_WIN32)
        if (socket == INVALID_SOCKET) {
            return std::nullopt;
        }
#else
        if (socket < 0) {
            return std::nullopt;
        }
#endif

        std::uint32_t payloadSize = 0;
        const auto headerBytes = ::recv(socket, reinterpret_cast<char*>(&payloadSize), sizeof(payloadSize), MSG_WAITALL);
#if defined(_WIN32)
        if (headerBytes == SOCKET_ERROR || headerBytes == 0) {
            platform::close_socket(socket);
            return std::nullopt;
        }
#else
        if (headerBytes <= 0) {
            platform::close_socket(socket);
            return std::nullopt;
        }
#endif

        std::array<std::byte, kPacketHeaderSize> headerBuffer{};
        const auto headerResult = ::recv(socket,
                                         reinterpret_cast<char*>(headerBuffer.data()),
                                         static_cast<int>(headerBuffer.size()),
                                         MSG_WAITALL);
#if defined(_WIN32)
        if (headerResult == SOCKET_ERROR || headerResult == 0) {
            platform::close_socket(socket);
            return std::nullopt;
        }
#else
        if (headerResult <= 0) {
            platform::close_socket(socket);
            return std::nullopt;
        }
#endif
        if (headerResult != static_cast<int>(headerBuffer.size())) {
            platform::close_socket(socket);
            return std::nullopt;
        }

        Packet packet;
        packet.header = decode_header(std::span<const std::byte, kPacketHeaderSize>(headerBuffer.data(), headerBuffer.size()));
        packet.payload.resize(payloadSize);
        if (payloadSize > 0) {
            const auto bodyBytes = ::recv(socket,
                                          reinterpret_cast<char*>(packet.payload.data()),
                                          static_cast<int>(payloadSize),
                                          MSG_WAITALL);
#if defined(_WIN32)
            if (bodyBytes == SOCKET_ERROR || static_cast<std::uint32_t>(bodyBytes) != payloadSize) {
                platform::close_socket(socket);
                return std::nullopt;
            }
#else
            if (bodyBytes <= 0 || static_cast<std::uint32_t>(bodyBytes) != payloadSize) {
                platform::close_socket(socket);
                return std::nullopt;
            }
#endif
        }

        platform::close_socket(socket);
        Endpoint endpoint = Endpoint::from_ipv4(client.sin_addr.s_addr, client.sin_port);
        return std::make_optional(std::make_pair(endpoint, std::move(packet)));
    });
}

} // namespace soul::net
