#pragma once

#include <memory>
#include <optional>

#include "Async/Task.h"
#include "Networking/Transport.h"

namespace soul::net {

class TcpTransport final : public Transport {
public:
    explicit TcpTransport(std::shared_ptr<soul::async::TaskScheduler> scheduler);
    ~TcpTransport() override;

    bool bind(const Endpoint& endpoint) override;
    void close() override;

    soul::async::Task<void> send(const Endpoint& endpoint, Packet packet) override;
    soul::async::Task<std::optional<std::pair<Endpoint, Packet>>> receive() override;

private:
    std::shared_ptr<soul::async::TaskScheduler> m_scheduler;
    int m_listenSocket{-1};
    bool m_isWindows{false};

    bool ensure_socket_created();
    void initialize_platform();
};

} // namespace soul::net
