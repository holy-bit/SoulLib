#pragma once

#include <functional>
#include <memory>

#include "Async/Task.h"
#include "Networking/Endpoint.h"
#include "Networking/Packet.h"

namespace soul::net {

class Transport {
public:
    virtual ~Transport() = default;

    virtual bool bind(const Endpoint& endpoint) = 0;
    virtual void close() = 0;

    virtual soul::async::Task<void> send(const Endpoint& endpoint, Packet packet) = 0;
    virtual soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> receive() = 0;
};

} // namespace soul::net
