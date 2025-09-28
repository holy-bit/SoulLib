#include "Networking/Endpoint.h"

#include <cstring>
#include <string>

#include "Networking/ByteOrder.h"
#include "Networking/Platform.h"

namespace soul::net {

Endpoint Endpoint::from_ipv4(std::uint32_t addressNetworkOrder, std::uint16_t portNetworkOrder) noexcept {
    Endpoint endpoint;
    endpoint.address = addressNetworkOrder;
    endpoint.port = portNetworkOrder;
    return endpoint;
}

Endpoint Endpoint::from_string(std::string_view dottedAddress, std::uint16_t hostPort) {
    Endpoint endpoint;
    if (!platform::startup()) {
        return endpoint;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(hostPort);
    if (inet_pton(AF_INET, std::string(dottedAddress).c_str(), &addr.sin_addr) != 1) {
        return endpoint;
    }

    endpoint.address = addr.sin_addr.s_addr;
    endpoint.port = addr.sin_port;
    return endpoint;
}

std::string Endpoint::to_string() const {
    if (!platform::startup()) {
        return "0.0.0.0:0";
    }
    char buffer[INET_ADDRSTRLEN];
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = address;
    if (inet_ntop(AF_INET, &addr.sin_addr, buffer, sizeof(buffer)) == nullptr) {
        return "0.0.0.0:0";
    }
    return std::string(buffer) + ":" + std::to_string(port_host_order());
}

std::uint32_t Endpoint::address_host_order() const noexcept {
    return byteorder::network_to_host(address);
}

std::uint16_t Endpoint::port_host_order() const noexcept {
    return ntohs(port);
}

} // namespace soul::net
