#pragma once

#include "LogLevel.h"
#include "LogEntry.h"

class LogChannel {
public:
    virtual ~LogChannel() = default;
    virtual void Write(const LogEntry& entry) = 0;
    void SetLevel(LogLevel lvl);
    LogLevel GetLevel() const;

private:
    LogLevel level;
};