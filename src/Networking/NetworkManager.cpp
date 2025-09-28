#include "Networking/NetworkManager.h"

#include <algorithm>
#include <cstdint>
#include <thread>
#include <vector>

namespace soul::net {

namespace {

constexpr bool sequence_greater(std::uint32_t lhs, std::uint32_t rhs) noexcept {
    return static_cast<std::int32_t>(lhs - rhs) > 0;
}

constexpr bool sequence_less_equal(std::uint32_t lhs, std::uint32_t rhs) noexcept {
    return static_cast<std::int32_t>(lhs - rhs) <= 0;
}

} // namespace

NetworkManager::NetworkManager(std::shared_ptr<soul::async::TaskScheduler> scheduler,
                               std::shared_ptr<Transport> tcpTransport,
                               std::shared_ptr<Transport> udpTransport)
    : m_scheduler(std::move(scheduler)),
      m_tcp(std::move(tcpTransport)),
      m_udp(std::move(udpTransport)) {}

soul::async::Task<void> NetworkManager::send(const Endpoint& endpoint, Packet packet) {
    if (packet.header.guarantee == DeliveryGuarantee::Reliable && is_udp_reliable(packet.header.channel)) {
        co_await send_reliable_udp(endpoint, std::move(packet));
        co_return;
    }

    auto& transport = packet.header.guarantee == DeliveryGuarantee::Reliable ? m_tcp : m_udp;
    co_await transport->send(endpoint, std::move(packet));
}

soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> NetworkManager::receive() {
    if (auto reliable = co_await m_tcp->receive(); reliable.has_value()) {
        co_return reliable;
    }

    auto udpPacket = co_await m_udp->receive();
    if (!udpPacket.has_value()) {
        co_return std::nullopt;
    }

    auto& [endpoint, packet] = udpPacket.value();

    if (is_udp_reliable(packet.header.channel)) {
        const bool ackOnly = has_flag(packet.header.flags, PacketFlags::Ack) && packet.payload.empty();

        record_ack(endpoint, packet.header);

        bool duplicate = false;
        if (!ackOnly) {
            duplicate = handle_incoming_sequence(endpoint, packet.header);
        }

        if (!ackOnly) {
            maybe_send_ack(endpoint, packet.header.channel);
        }

        if (duplicate || ackOnly) {
            co_return std::nullopt;
        }
    }

    co_return udpPacket;
}

void NetworkManager::enable_udp_reliability(std::uint16_t channel, bool enabled) {
    std::lock_guard lock(m_mutex);
    m_udpReliableChannels[channel] = enabled;
}

void NetworkManager::configure_udp_retransmission(std::chrono::milliseconds interval, std::uint8_t maxAttempts) {
    std::lock_guard lock(m_mutex);
    m_retransmitInterval = interval;
    m_maxRetransmitAttempts = std::max<std::uint8_t>(1, maxAttempts);
}

std::uint64_t NetworkManager::make_connection_key(const Endpoint& endpoint) noexcept {
    const auto address = static_cast<std::uint64_t>(endpoint.address_host_order());
    const auto port = static_cast<std::uint64_t>(endpoint.port_host_order());
    return (address << 16) | port;
}

bool NetworkManager::is_udp_reliable(std::uint16_t channel) {
    std::lock_guard lock(m_mutex);
    auto it = m_udpReliableChannels.find(channel);
    return it != m_udpReliableChannels.end() && it->second;
}

soul::async::Task<void> NetworkManager::send_reliable_udp(const Endpoint& endpoint, Packet packet) {
    PendingPacket pendingCopy;
    std::uint32_t sequence;

    {
        std::lock_guard lock(m_mutex);
        auto& connection = m_connections[make_connection_key(endpoint)];
        auto& channelState = connection.channels[packet.header.channel];
        sequence = ++channelState.nextOutgoingSequence;
        packet.header.sequence = sequence;
        packet.header.acknowledgment = channelState.lastReceivedSequence;
        packet.header.acknowledgmentMask = channelState.receivedMask;

        pendingCopy.endpoint = endpoint;
        pendingCopy.packet = packet;
        pendingCopy.lastSent = std::chrono::steady_clock::now();
        pendingCopy.attempts = 0;

        channelState.pending[sequence] = pendingCopy;
    }

    co_await m_udp->send(endpoint, std::move(packet));
    schedule_retransmission(endpoint, pendingCopy.packet.header.channel, sequence);
}

void NetworkManager::record_ack(const Endpoint& endpoint, const PacketHeader& header) {
    std::lock_guard lock(m_mutex);
    auto connectionIt = m_connections.find(make_connection_key(endpoint));
    if (connectionIt == m_connections.end()) {
        return;
    }

    auto channelIt = connectionIt->second.channels.find(header.channel);
    if (channelIt == connectionIt->second.channels.end()) {
        return;
    }

    auto& pending = channelIt->second.pending;
    std::vector<std::uint32_t> sequencesToErase;
    for (auto& [sequence, entry] : pending) {
        if (is_sequence_acked(sequence, header.acknowledgment, header.acknowledgmentMask)) {
            sequencesToErase.emplace_back(sequence);
        }
    }
    for (auto seq : sequencesToErase) {
        pending.erase(seq);
    }
}

bool NetworkManager::handle_incoming_sequence(const Endpoint& endpoint, const PacketHeader& header) {
    std::lock_guard lock(m_mutex);
    auto& connection = m_connections[make_connection_key(endpoint)];
    auto& channelState = connection.channels[header.channel];

    if (!channelState.hasReceived) {
        channelState.hasReceived = true;
        channelState.lastReceivedSequence = header.sequence;
        channelState.receivedMask = 0;
        channelState.pendingAck = true;
        return false;
    }

    if (sequence_greater(header.sequence, channelState.lastReceivedSequence)) {
        auto diff = header.sequence - channelState.lastReceivedSequence;
        if (diff >= 32) {
            channelState.receivedMask = 0;
        } else {
            channelState.receivedMask <<= diff;
        }
        channelState.lastReceivedSequence = header.sequence;
        channelState.pendingAck = true;
        return false;
    }

    auto diff = channelState.lastReceivedSequence - header.sequence;
    if (diff == 0) {
        return true;
    }
    if (diff > 32) {
        return true;
    }

    const auto bit = 1u << (diff - 1);
    if ((channelState.receivedMask & bit) != 0) {
        return true;
    }

    channelState.receivedMask |= bit;
    channelState.pendingAck = true;
    return false;
}

void NetworkManager::maybe_send_ack(const Endpoint& endpoint, std::uint16_t channel) {
    PendingPacket ackPacket;
    bool shouldSend = false;

    {
        std::lock_guard lock(m_mutex);
        auto connectionIt = m_connections.find(make_connection_key(endpoint));
        if (connectionIt == m_connections.end()) {
            return;
        }
        auto channelIt = connectionIt->second.channels.find(channel);
        if (channelIt == connectionIt->second.channels.end()) {
            return;
        }

        auto& channelState = channelIt->second;
        if (!channelState.pendingAck) {
            return;
        }

        channelState.pendingAck = false;

        ackPacket.endpoint = endpoint;
        ackPacket.packet.header.channel = channel;
        ackPacket.packet.header.guarantee = DeliveryGuarantee::Unreliable;
        ackPacket.packet.header.flags = PacketFlags::Ack;
        ackPacket.packet.header.sequence = 0;
        ackPacket.packet.header.acknowledgment = channelState.lastReceivedSequence;
        ackPacket.packet.header.acknowledgmentMask = channelState.receivedMask;
        ackPacket.packet.payload.clear();
        shouldSend = true;
    }

    if (shouldSend) {
        auto task = m_udp->send(endpoint, std::move(ackPacket.packet));
        m_scheduler->schedule(std::move(task));
    }
}

void NetworkManager::schedule_retransmission(const Endpoint& endpoint,
                                             std::uint16_t channel,
                                             std::uint32_t sequence) {
    auto task = retransmit_after(endpoint, channel, sequence);
    m_scheduler->schedule(std::move(task));
}

soul::async::Task<void> NetworkManager::retransmit_after(Endpoint endpoint,
                                                         std::uint16_t channel,
                                                         std::uint32_t sequence) {
    std::chrono::milliseconds waitDuration;
    std::uint8_t maxAttempts;

    {
        std::lock_guard lock(m_mutex);
        waitDuration = m_retransmitInterval;
        maxAttempts = m_maxRetransmitAttempts;
    }

    co_await m_scheduler->run_async([waitDuration]() {
        std::this_thread::sleep_for(waitDuration);
    });

    PendingPacket packetToResend;
    bool shouldRetry = false;

    {
        std::lock_guard lock(m_mutex);
        auto connectionIt = m_connections.find(make_connection_key(endpoint));
        if (connectionIt == m_connections.end()) {
            co_return;
        }
        auto channelIt = connectionIt->second.channels.find(channel);
        if (channelIt == connectionIt->second.channels.end()) {
            co_return;
        }

        auto pendingIt = channelIt->second.pending.find(sequence);
        if (pendingIt == channelIt->second.pending.end()) {
            co_return;
        }

        if (pendingIt->second.attempts + 1 >= maxAttempts) {
            channelIt->second.pending.erase(pendingIt);
            co_return;
        }

        auto& pendingEntry = pendingIt->second;
        pendingEntry.attempts += 1;
        pendingEntry.lastSent = std::chrono::steady_clock::now();
        pendingEntry.packet.header.acknowledgment = channelIt->second.lastReceivedSequence;
        pendingEntry.packet.header.acknowledgmentMask = channelIt->second.receivedMask;
        packetToResend = pendingEntry;
        shouldRetry = true;
    }

    if (shouldRetry) {
        co_await m_udp->send(packetToResend.endpoint, packetToResend.packet);
        schedule_retransmission(endpoint, channel, sequence);
    }
}

bool NetworkManager::is_sequence_acked(std::uint32_t sequence,
                                       std::uint32_t ack,
                                       std::uint32_t ackMask) {
    if (sequence == ack) {
        return true;
    }
    if (!sequence_less_equal(sequence, ack)) {
        return false;
    }
    auto diff = ack - sequence;
    if (diff == 0) {
        return true;
    }
    if (diff > 32) {
        return false;
    }
    const auto bit = 1u << (diff - 1);
    return (ackMask & bit) != 0;
}

} // namespace soul::net
