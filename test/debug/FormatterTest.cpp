#include "../../include/debug/Formatter.h"
#include <gtest/gtest.h>

TEST(FormatterTest, FormatLogEntry) {
    Formatter formatter;
    formatter.SetPattern("[LEVEL]");
    LogEntry entry{LogLevel::Debug, "Test message", {}, {}};
    std::string formatted = formatter.Format(entry);
    EXPECT_EQ(formatted, "[LEVEL]: Test message");
}