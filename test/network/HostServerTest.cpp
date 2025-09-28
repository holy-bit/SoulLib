#include "network/host/HostServer.h"
#include <gtest/gtest.h>

TEST(HostServerTest, CanInstantiate) {
    // Should compile and construct without error
    HostServer server;
    (void)server;
}

// Further behavior tests (e.g., network interactions) can be added once shutdown support is available.
