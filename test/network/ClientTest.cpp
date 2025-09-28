#include "network/client/Client.h"
#include <gtest/gtest.h>

TEST(ClientTest, ConnectInvalidAddress) {
    // Connecting to an invalid IP should fail
    Client client("256.256.256.256", 12345);
    EXPECT_FALSE(client.connectToServer());
}

TEST(ClientTest, ConnectNoServerRunning) {
    // Assuming no server on this port, connect should fail
    Client client("127.0.0.1", 65000);
    EXPECT_FALSE(client.connectToServer());
}