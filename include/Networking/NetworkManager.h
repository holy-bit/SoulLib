#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "Async/Task.h"
#include "Networking/Packet.h"
#include "Networking/Transport.h"

namespace soul::net {

/**
 * @brief High level orchestrator for dual (UDP/TCP) transport with reliability metadata.
 */
class NetworkManager {
public:
    /**
     * @brief Constructs a manager bound to shared transports and scheduler infrastructure.
     * @param scheduler Worker pool used to run retransmissions and async socket operations.
     * @param tcpTransport Reliable stream transport (e.g., TCP).
     * @param udpTransport Datagram transport (e.g., UDP) leveraged for unreliable and reliable-UDP traffic.
     */
    NetworkManager(std::shared_ptr<soul::async::TaskScheduler> scheduler,
                   std::shared_ptr<Transport> tcpTransport,
                   std::shared_ptr<Transport> udpTransport);

    /**
     * @brief Sends a packet using the transport that matches its delivery guarantee.
     * @param endpoint Remote endpoint to target.
     * @param packet Payload + metadata to transmit. Ownership transfers to the scheduler.
     */
    soul::async::Task<void> send(const Endpoint& endpoint, Packet packet);
    /**
     * @brief Polls all transports for the next available packet.
     * @return Optional tuple of origin endpoint and received packet. `std::nullopt` when idle.
     */
    soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> receive();

    /**
     * @brief Enables or disables UDP-layer retransmission for the given logical channel.
     *
     * When enabled, packets sent with `DeliveryGuarantee::Reliable` on this channel ride on top of
     * the UDP transport using sequence/ACK metadata. Retransmissions are orchestrated via the
     * shared task scheduler.
     */
    void enable_udp_reliability(std::uint16_t channel, bool enabled);

    /**
     * @brief Configures timeout and retry budget for UDP retransmissions.
     * @param interval Delay before retrying pending packets.
     * @param maxAttempts Total attempts (initial send + retries) before the packet is dropped.
     */
    void configure_udp_retransmission(std::chrono::milliseconds interval, std::uint8_t maxAttempts);

private:
    struct PendingPacket {
        Endpoint endpoint;
        Packet packet;
        std::chrono::steady_clock::time_point lastSent;
        std::uint8_t attempts{0};
    };

    struct ReliableChannelState {
        std::uint32_t nextOutgoingSequence{0};
        std::uint32_t lastReceivedSequence{0};
        std::uint32_t receivedMask{0};
        bool hasReceived{false};
        std::unordered_map<std::uint32_t, PendingPacket> pending;
        bool pendingAck{false};
    };

    struct ConnectionReliabilityState {
        std::unordered_map<std::uint16_t, ReliableChannelState> channels;
    };

    std::shared_ptr<soul::async::TaskScheduler> m_scheduler;
    std::shared_ptr<Transport> m_tcp;
    std::shared_ptr<Transport> m_udp;
    std::unordered_map<std::uint16_t, bool> m_udpReliableChannels;
    std::unordered_map<std::uint64_t, ConnectionReliabilityState> m_connections;
    std::chrono::milliseconds m_retransmitInterval{std::chrono::milliseconds(75)};
    std::uint8_t m_maxRetransmitAttempts{5};
    std::mutex m_mutex;

    static std::uint64_t make_connection_key(const Endpoint& endpoint) noexcept;
    bool is_udp_reliable(std::uint16_t channel);
    soul::async::Task<void> send_reliable_udp(const Endpoint& endpoint, Packet packet);
    void record_ack(const Endpoint& endpoint, const PacketHeader& header);
    bool handle_incoming_sequence(const Endpoint& endpoint, const PacketHeader& header);
    void maybe_send_ack(const Endpoint& endpoint, std::uint16_t channel);
    void schedule_retransmission(const Endpoint& endpoint, std::uint16_t channel, std::uint32_t sequence);
    soul::async::Task<void> retransmit_after(Endpoint endpoint, std::uint16_t channel, std::uint32_t sequence);
    static bool is_sequence_acked(std::uint32_t sequence, std::uint32_t ack, std::uint32_t ackMask);
};

} // namespace soul::net
