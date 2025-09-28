#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <vector>

namespace soul::net {

/**
 * @brief Delivery contract requested for an outgoing packet.
 */
enum class DeliveryGuarantee : std::uint8_t {
    Unreliable,
    Reliable,
};

/**
 * @brief Flags piggybacked with every packet header.
 */
enum class PacketFlags : std::uint8_t {
    None = 0x00,
    Ack = 0x01,
};

constexpr PacketFlags operator|(PacketFlags lhs, PacketFlags rhs) noexcept {
    return static_cast<PacketFlags>(static_cast<std::uint8_t>(lhs) |
                                    static_cast<std::uint8_t>(rhs));
}

constexpr PacketFlags operator&(PacketFlags lhs, PacketFlags rhs) noexcept {
    return static_cast<PacketFlags>(static_cast<std::uint8_t>(lhs) &
                                    static_cast<std::uint8_t>(rhs));
}

constexpr bool has_flag(PacketFlags value, PacketFlags flag) noexcept {
    return (value & flag) != PacketFlags::None;
}

struct PacketHeader {
    std::uint32_t sequence{0};
    std::uint32_t acknowledgment{0};
    std::uint32_t acknowledgmentMask{0};
    DeliveryGuarantee guarantee{DeliveryGuarantee::Unreliable};
    PacketFlags flags{PacketFlags::None};
    std::uint16_t channel{0};
};

struct Packet {
    PacketHeader header;
    std::vector<std::byte> payload;
};

inline constexpr std::size_t kPacketHeaderSize = sizeof(PacketHeader);

inline std::array<std::byte, kPacketHeaderSize> encode_header(const PacketHeader& header) {
    std::array<std::byte, kPacketHeaderSize> buffer{};
    std::memcpy(buffer.data(), &header, sizeof(PacketHeader));
    return buffer;
}

inline PacketHeader decode_header(std::span<const std::byte, kPacketHeaderSize> buffer) {
    PacketHeader header{};
    std::memcpy(&header, buffer.data(), sizeof(PacketHeader));
    return header;
}

} // namespace soul::net
