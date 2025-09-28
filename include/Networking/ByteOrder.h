#pragma once

#include <cstdint>

namespace soul::net::byteorder {

inline std::uint16_t host_to_network(std::uint16_t value) noexcept {
#if defined(_WIN32)
    return _byteswap_ushort(value);
#else
    return htobe16(value);
#endif
}

inline std::uint32_t host_to_network(std::uint32_t value) noexcept {
#if defined(_WIN32)
    return _byteswap_ulong(value);
#else
    return htobe32(value);
#endif
}

inline std::uint16_t network_to_host(std::uint16_t value) noexcept {
#if defined(_WIN32)
    return _byteswap_ushort(value);
#else
    return be16toh(value);
#endif
}

inline std::uint32_t network_to_host(std::uint32_t value) noexcept {
#if defined(_WIN32)
    return _byteswap_ulong(value);
#else
    return be32toh(value);
#endif
}

} // namespace soul::net::byteorder
