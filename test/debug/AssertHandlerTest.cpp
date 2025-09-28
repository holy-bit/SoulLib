#include "../../include/debug/AssertHandler.h"
#include <gtest/gtest.h>

TEST(AssertHandlerTest, CheckCondition) {
    AssertHandler handler;
    bool callbackCalled = false;
    handler.SetFailureCallback([&](const LogEntry& entry) {
        callbackCalled = true;
    });

    handler.Check(false, "Test failure", {"file.cpp", 42, "function"});
    EXPECT_TRUE(callbackCalled);
}