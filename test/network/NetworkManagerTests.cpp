#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "Async/Task.h"
#include "Networking/NetworkManager.h"
#include "Networking/UdpTransport.h"
#include "Networking/TcpTransport.h"

namespace {

constexpr int kPortA = 35555;
constexpr int kPortB = 35556;

class DroppingUdpTransport final : public soul::net::Transport {
public:
    DroppingUdpTransport(std::shared_ptr<soul::async::TaskScheduler> scheduler, std::size_t drops)
        : m_inner(std::make_shared<soul::net::UdpTransport>(std::move(scheduler))),
          m_dropsRemaining(drops) {}

    bool bind(const soul::net::Endpoint& endpoint) override {
        return m_inner->bind(endpoint);
    }

    void close() override {
        m_inner->close();
    }

    soul::async::Task<void> send(const soul::net::Endpoint& endpoint, soul::net::Packet packet) override {
        if (m_dropsRemaining > 0) {
            --m_dropsRemaining;
            co_return;
        }
        co_await m_inner->send(endpoint, std::move(packet));
    }

    soul::async::Task<std::optional<std::pair<soul::net::Endpoint, soul::net::Packet>>>
    receive() override {
        co_return co_await m_inner->receive();
    }

private:
    std::shared_ptr<soul::net::UdpTransport> m_inner;
    std::size_t m_dropsRemaining{0};
};

} // namespace

TEST(NetworkManager, UdpSendAndReceive) {
    auto scheduler = std::make_shared<soul::async::TaskScheduler>(4);

    auto tcpA = std::make_shared<soul::net::TcpTransport>(scheduler);
    auto tcpB = std::make_shared<soul::net::TcpTransport>(scheduler);

    auto udpA = std::make_shared<soul::net::UdpTransport>(scheduler);
    auto udpB = std::make_shared<soul::net::UdpTransport>(scheduler);

    ASSERT_TRUE(udpA->bind(soul::net::Endpoint::from_string("127.0.0.1", kPortA)));
    ASSERT_TRUE(udpB->bind(soul::net::Endpoint::from_string("127.0.0.1", kPortB)));

    soul::net::NetworkManager managerA(scheduler, tcpA, udpA);
    soul::net::NetworkManager managerB(scheduler, tcpB, udpB);

    std::cout << "launching receive future" << std::endl;
    auto receiveFuture = std::async(std::launch::async, [&]() {
        std::cout << "receive coroutine starting" << std::endl;
        return managerB.receive().get();
    });

    soul::net::Packet packet;
    packet.header.channel = 1;
    packet.header.guarantee = soul::net::DeliveryGuarantee::Unreliable;
    packet.payload = {std::byte{'S'}, std::byte{'O'}, std::byte{'U'}, std::byte{'L'}};

    auto destination = soul::net::Endpoint::from_string("127.0.0.1", kPortB);
    std::cout << "sending packet" << std::endl;
    managerA.send(destination, std::move(packet)).get();
    std::cout << "send completed" << std::endl;

    std::cout << "waiting for receive" << std::endl;
    auto result = receiveFuture.get();
    std::cout << "receive completed" << std::endl;
    ASSERT_TRUE(result.has_value());
    auto [endpoint, receivedPacket] = std::move(result.value());

    EXPECT_EQ(endpoint.port_host_order(), kPortA);
    ASSERT_EQ(receivedPacket.payload.size(), 4u);
    EXPECT_EQ(receivedPacket.payload[0], std::byte{'S'});
    EXPECT_EQ(receivedPacket.payload[1], std::byte{'O'});
    EXPECT_EQ(receivedPacket.payload[2], std::byte{'U'});
    EXPECT_EQ(receivedPacket.payload[3], std::byte{'L'});
}

TEST(NetworkManager, UdpReliableRetransmission) {
    auto scheduler = std::make_shared<soul::async::TaskScheduler>(4);

    auto tcpA = std::make_shared<soul::net::TcpTransport>(scheduler);
    auto tcpB = std::make_shared<soul::net::TcpTransport>(scheduler);

    auto udpA = std::make_shared<DroppingUdpTransport>(scheduler, 1);
    auto udpB = std::make_shared<soul::net::UdpTransport>(scheduler);

    ASSERT_TRUE(udpA->bind(soul::net::Endpoint::from_string("127.0.0.1", kPortA + 2)));
    ASSERT_TRUE(udpB->bind(soul::net::Endpoint::from_string("127.0.0.1", kPortB + 2)));

    soul::net::NetworkManager managerA(scheduler, tcpA, udpA);
    soul::net::NetworkManager managerB(scheduler, tcpB, udpB);

    constexpr std::uint16_t channel = 3;
    managerA.enable_udp_reliability(channel, true);
    managerB.enable_udp_reliability(channel, true);
    managerA.configure_udp_retransmission(std::chrono::milliseconds(40), 5);

    std::cout << "launching reliable receive future" << std::endl;
    auto receiveFuture = std::async(std::launch::async, [&]() {
        std::cout << "reliable receive coroutine starting" << std::endl;
        return managerB.receive().get();
    });

    soul::net::Packet packet;
    packet.header.channel = channel;
    packet.header.guarantee = soul::net::DeliveryGuarantee::Reliable;
    packet.payload = {std::byte{'R'}, std::byte{'E'}, std::byte{'L'}, std::byte{'I'}};

    auto destination = soul::net::Endpoint::from_string("127.0.0.1", kPortB + 2);
    std::cout << "sending reliable packet" << std::endl;
    managerA.send(destination, packet).get();
    std::cout << "reliable send completed" << std::endl;

    std::cout << "waiting for reliable receive" << std::endl;
    auto result = receiveFuture.get();
    std::cout << "reliable receive completed" << std::endl;
    ASSERT_TRUE(result.has_value());
    auto [endpoint, receivedPacket] = std::move(result.value());

    EXPECT_EQ(receivedPacket.payload.size(), 4u);
    EXPECT_EQ(receivedPacket.payload[0], std::byte{'R'});
    EXPECT_EQ(receivedPacket.payload[1], std::byte{'E'});
    EXPECT_EQ(receivedPacket.payload[2], std::byte{'L'});
    EXPECT_EQ(receivedPacket.payload[3], std::byte{'I'});

    // Allow time for ACK to propagate so that retransmission backlog clears without exceeding retries.
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}
