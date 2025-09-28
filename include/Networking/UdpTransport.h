#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "Async/Task.h"
#include "Networking/Transport.h"

namespace soul::net {

class UdpTransport final : public Transport {
public:
    explicit UdpTransport(std::shared_ptr<soul::async::TaskScheduler> scheduler);
    ~UdpTransport() override;

    bool bind(const Endpoint& endpoint) override;
    void close() override;

    soul::async::Task<void> send(const Endpoint& endpoint, Packet packet) override;
    soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> receive() override;

private:
    std::shared_ptr<soul::async::TaskScheduler> m_scheduler;
    int m_socket{-1};
    bool m_isWindows{false};

    bool ensure_socket_created();
    void initialize_platform();
};

} // namespace soul::net
