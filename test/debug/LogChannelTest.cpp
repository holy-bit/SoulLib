#include "../../include/debug/LogChannel.h"
#include <gtest/gtest.h>

class TestLogChannel : public LogChannel {
public:
    void Write(const LogEntry& entry) override {
        lastEntry = entry;
    }

    LogEntry lastEntry;
};

TEST(LogChannelTest, SetAndGetLevel) {
    TestLogChannel channel;
    channel.SetLevel(LogLevel::Warning);
    EXPECT_EQ(channel.GetLevel(), LogLevel::Warning);
}

TEST(LogChannelTest, WriteLogEntry) {
    TestLogChannel channel;
    LogEntry entry{
        LogLevel::Error, 
        "Test message", 
        {"test_file.cpp", 42, "TestFunction"}, // Provide valid SourceInfo
        Timestamp::Now() // Provide valid Timestamp
    };
    channel.Write(entry);
    EXPECT_EQ(channel.lastEntry.message, "Test message");
    EXPECT_EQ(channel.lastEntry.level, LogLevel::Error);
    EXPECT_EQ(channel.lastEntry.source.file, "test_file.cpp"); // Verify SourceInfo
    EXPECT_EQ(channel.lastEntry.source.line, 42);
    EXPECT_EQ(channel.lastEntry.source.function, "TestFunction");
}

TEST(LogEntryTest, CopyAssignment) {
    LogEntry entry1{
        LogLevel::Info, 
        "Original message", 
        {"original_file.cpp", 10, "OriginalFunction"},
        Timestamp::Now()
    };

    LogEntry entry2 = entry1; // Test copy assignment

    EXPECT_EQ(entry2.message, "Original message");
    EXPECT_EQ(entry2.level, LogLevel::Info);
    EXPECT_EQ(entry2.source.file, "original_file.cpp");
    EXPECT_EQ(entry2.source.line, 10);
    EXPECT_EQ(entry2.source.function, "OriginalFunction");
}

TEST(LogEntryTest, MoveAssignment) {
    LogEntry entry1{
        LogLevel::Debug, 
        "Temporary message", 
        {"temp_file.cpp", 20, "TempFunction"},
        Timestamp::Now()
    };

    LogEntry entry2 = std::move(entry1); // Test move assignment

    EXPECT_EQ(entry2.message, "Temporary message");
    EXPECT_EQ(entry2.level, LogLevel::Debug);
    EXPECT_EQ(entry2.source.file, "temp_file.cpp");
    EXPECT_EQ(entry2.source.line, 20);
    EXPECT_EQ(entry2.source.function, "TempFunction");
}