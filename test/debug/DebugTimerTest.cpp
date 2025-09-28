#include "../../include/debug/DebugTimer.h"
#include <gtest/gtest.h>

TEST(DebugTimerTest, StartAndStop) {
    DebugTimer timer;
    timer.Start();
    timer.Stop();
    EXPECT_GE(timer.Elapsed().count(), 0);
}

TEST(DebugTimerTest, Reset) {
    DebugTimer timer;
    timer.Start();
    timer.Stop();
    timer.Reset();
    EXPECT_EQ(timer.Elapsed().count(), 0.0); // Fix: Ensure the elapsed time is reset to 0
}