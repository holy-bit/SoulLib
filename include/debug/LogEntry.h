#pragma once

#include <string>
#include "LogLevel.h"
#include "SourceInfo.h"
#include "Timestamp.h"

struct LogEntry {
    LogLevel level;
    std::string message;
    SourceInfo source;
    Timestamp timestamp;
};