#include "Networking/UdpTransport.h"

#include <array>
#include <cerrno>
#include <optional>
#include <vector>

#include "Networking/Platform.h"

namespace soul::net {

namespace {

platform::SocketHandle create_udp_socket() {
    if (!platform::startup()) {
        return platform::invalid_socket;
    }

    platform::SocketHandle socketHandle{};
#if defined(_WIN32)
    socketHandle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    socketHandle = ::socket(AF_INET, SOCK_DGRAM, 0);
#endif
    return socketHandle;
}

}

UdpTransport::UdpTransport(std::shared_ptr<soul::async::TaskScheduler> scheduler)
    : m_scheduler(std::move(scheduler)) {}

UdpTransport::~UdpTransport() {
    close();
}

bool UdpTransport::ensure_socket_created() {
    if (m_socket != platform::invalid_socket) {
        return true;
    }

    m_socket = create_udp_socket();
    return m_socket != platform::invalid_socket;
}

void UdpTransport::initialize_platform() {
#if defined(_WIN32)
    m_isWindows = true;
#endif
}

bool UdpTransport::bind(const Endpoint& endpoint) {
    initialize_platform();
    if (!ensure_socket_created()) {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = endpoint.port;
    addr.sin_addr.s_addr = endpoint.address;

    const auto result = ::bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
#if defined(_WIN32)
    return result != SOCKET_ERROR;
#else
    return result == 0;
#endif
}

void UdpTransport::close() {
    if (m_socket != platform::invalid_socket) {
        platform::close_socket(m_socket);
        m_socket = platform::invalid_socket;
    }
}

soul::async::Task<void> UdpTransport::send(const Endpoint& endpoint, Packet packet) {
    if (!ensure_socket_created()) {
        co_return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = endpoint.port;
    addr.sin_addr.s_addr = endpoint.address;

    auto scheduler = m_scheduler;
    co_await scheduler->run_async([socket = m_socket, addr, packet = std::move(packet)]() mutable {
        auto headerBytes = encode_header(packet.header);
        std::vector<std::byte> buffer;
        buffer.reserve(headerBytes.size() + packet.payload.size());
        buffer.insert(buffer.end(), headerBytes.begin(), headerBytes.end());
        buffer.insert(buffer.end(), packet.payload.begin(), packet.payload.end());

        ::sendto(socket,
                 reinterpret_cast<const char*>(buffer.data()),
                 static_cast<int>(buffer.size()),
                 0,
                 reinterpret_cast<const sockaddr*>(&addr),
                 sizeof(addr));
    });
}

soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> UdpTransport::receive() {
    if (!ensure_socket_created()) {
        co_return std::nullopt;
    }

    auto scheduler = m_scheduler;
    co_return co_await scheduler->run_async([socket = m_socket]() -> std::optional<std::pair<Endpoint, Packet>> {
    std::array<std::byte, 1500> buffer{};
    sockaddr_in from{};
#if defined(_WIN32)
    int fromLen = static_cast<int>(sizeof(from));
#else
    socklen_t fromLen = sizeof(from);
#endif

        const auto received = ::recvfrom(socket,
                                         reinterpret_cast<char*>(buffer.data()),
                                         static_cast<int>(buffer.size()),
                                         0,
                                         reinterpret_cast<sockaddr*>(&from),
                     &fromLen);
#if defined(_WIN32)
        if (received == SOCKET_ERROR) {
            return std::nullopt;
        }
#else
        if (received < 0) {
            return std::nullopt;
        }
#endif
        if (received < static_cast<int>(kPacketHeaderSize)) {
            return std::nullopt;
        }

        std::span<const std::byte, kPacketHeaderSize> headerSpan(
            reinterpret_cast<const std::byte*>(buffer.data()), kPacketHeaderSize);
        Packet packet;
        packet.header = decode_header(headerSpan);
        packet.payload.assign(buffer.begin() + kPacketHeaderSize, buffer.begin() + received);
        Endpoint endpoint = Endpoint::from_ipv4(from.sin_addr.s_addr, from.sin_port);
        return std::make_optional(std::make_pair(endpoint, std::move(packet)));
    });
}

} // namespace soul::net
