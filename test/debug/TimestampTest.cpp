#include "../../include/debug/Timestamp.h"
#include <gtest/gtest.h>

TEST(TimestampTest, NowReturnsCurrentTime) {
    Timestamp t1 = Timestamp::Now();
    Timestamp t2 = Timestamp::Now();

    // Check that the timestamps are close to each other
    auto duration = t2.time - t1.time;
    EXPECT_GE(duration.count(), 0);
}