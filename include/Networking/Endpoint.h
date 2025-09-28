#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace soul::net {

/**
 * @brief Represents an IPv4 endpoint (address + port).
 */
struct Endpoint {
    std::uint32_t address{0}; ///< Stored in network byte order.
    std::uint16_t port{0};    ///< Stored in network byte order.

    [[nodiscard]] static Endpoint from_ipv4(std::uint32_t addressNetworkOrder, std::uint16_t portNetworkOrder) noexcept;
    [[nodiscard]] static Endpoint from_string(std::string_view dottedAddress, std::uint16_t hostPort);

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::uint32_t address_host_order() const noexcept;
    [[nodiscard]] std::uint16_t port_host_order() const noexcept;
};

} // namespace soul::net
