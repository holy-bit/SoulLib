#include "../../include/debug/Debug.h"
#include "../../include/debug/DebugConfig.h"
#include "../../include/debug/SourceInfo.h"
#include <gtest/gtest.h>

TEST(DebugAPITest, InitAndShutdown) {
    DebugConfig config;
    config.minLogLevel = LogLevel::Info;

    EXPECT_NO_THROW(Debug::Init(config));
    EXPECT_NO_THROW(Debug::Shutdown());
}

TEST(DebugAPITest, LogDebugMessage) {
    DebugConfig config;
    config.minLogLevel = LogLevel::Debug;
    Debug::Init(config);

    SourceInfo info{"file.cpp", 42, "function"};
    EXPECT_NO_THROW(Debug::Debug("Debug message", info));

    Debug::Shutdown();
}

TEST(DebugAPITest, LogWarningMessage) {
    DebugConfig config;
    config.minLogLevel = LogLevel::Warning;
    Debug::Init(config);

    SourceInfo info{"file.cpp", 42, "function"};
    EXPECT_NO_THROW(Debug::Warning("Warning message", info));

    Debug::Shutdown();
}

TEST(DebugAPITest, LogErrorMessage) {
    DebugConfig config;
    config.minLogLevel = LogLevel::Error;
    Debug::Init(config);

    SourceInfo info{"file.cpp", 42, "function"};
    EXPECT_NO_THROW(Debug::Error("Error message", info));

    Debug::Shutdown();
}