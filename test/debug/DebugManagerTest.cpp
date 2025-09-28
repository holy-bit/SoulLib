#include "../../include/debug/Debug.h"
#include "../../include/debug/DebugConfig.h"
#include "../../include/debug/LogChannel.h"
#include "../../include/debug/SourceInfo.h"
#include <gtest/gtest.h>
#include <memory>
#include <string>

class TestLogChannel : public LogChannel {
public:
    void Write(const LogEntry& entry) override {
        lastEntry = entry;
    }

    LogEntry lastEntry;
};

TEST(DebugManagerTest, InitAndShutdown) {
    Debug::Manager manager;
    DebugConfig config;
    config.minLogLevel = LogLevel::Info;
    manager.Init(config);

    // Verify initialization
    EXPECT_NO_THROW(manager.Init(config));

    manager.Shutdown();
    // Verify shutdown
    EXPECT_NO_THROW(manager.Shutdown());
}

TEST(DebugManagerTest, RegisterChannel) {
    Debug::Manager manager;
    auto channel = std::make_shared<TestLogChannel>();
    manager.RegisterChannel(channel);

    // Verify channel registration
    EXPECT_NO_THROW(manager.RegisterChannel(channel));
}

TEST(DebugManagerTest, LogMessage) {
    Debug::Manager manager;
    DebugConfig config;
    config.minLogLevel = LogLevel::Debug;
    manager.Init(config);

    auto channel = std::make_shared<TestLogChannel>();
    manager.RegisterChannel(channel);

    SourceInfo info{"file.cpp", 42, "function"};
    manager.Log(LogLevel::Debug, "Test message", info);

    // Verify logging
    EXPECT_EQ(channel->lastEntry.message, "Test message");
    EXPECT_EQ(channel->lastEntry.level, LogLevel::Debug);
    EXPECT_EQ(channel->lastEntry.source.file, "file.cpp");
    EXPECT_EQ(channel->lastEntry.source.line, 42);
    EXPECT_EQ(channel->lastEntry.source.function, "function");
}

TEST(DebugManagerTest, SetConfig) {
    Debug::Manager manager;
    DebugConfig config;
    config.minLogLevel = LogLevel::Warning;
    manager.SetConfig(config);

    // Verify configuration update
    EXPECT_NO_THROW(manager.SetConfig(config));
}