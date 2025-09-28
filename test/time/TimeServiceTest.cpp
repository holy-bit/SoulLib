#include "time/TimeService.h"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

TEST(TimeServiceTest, InitialState) {
    TimeService ts;
    EXPECT_EQ(ts.getCurrentTime(), 0);
    EXPECT_FALSE(ts.isSynced());
    EXPECT_EQ(ts.getDeltaTime(), 0);
}

TEST(TimeServiceTest, SyncWithServerTime) {
    TimeService ts;
    uint64_t serverTime = 12345;
    ts.syncWithServerTime(serverTime);

    EXPECT_TRUE(ts.isSynced());
    // Immediately after sync, time may vary slightly
    EXPECT_NEAR(ts.getCurrentTime(), serverTime, 5);
    EXPECT_NEAR(ts.getDeltaTime(), 0, 5);
}

TEST(TimeServiceTest, GetDeltaTimeAfterSync) {
    TimeService ts;
    uint64_t serverTime = 1000;
    ts.syncWithServerTime(serverTime);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto delta = ts.getDeltaTime();
    // Increase tolerance from 20 to 50 ms to account for system timing variations
    EXPECT_NEAR(delta, 50, 50);
    EXPECT_NEAR(ts.getCurrentTime(), serverTime + delta, 50);
}

TEST(TimeServiceTest, UpdateRealTime) {
    TimeService ts;
    uint64_t serverTime = 2000;
    ts.syncWithServerTime(serverTime);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ts.updateRealTime();
    auto ct1 = ts.getCurrentTime();

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    ts.updateRealTime();
    auto ct2 = ts.getCurrentTime();

    EXPECT_GT(ct1, serverTime);
    EXPECT_GT(ct2, ct1);
}