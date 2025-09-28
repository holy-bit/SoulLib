#pragma once

#include <string>
#include "LogLevel.h"

struct DebugConfig {
    LogLevel defaultLevel;
    LogLevel minLogLevel = LogLevel::Info;
    bool enableConsole;
    bool enableFile;
    std::string logFilePath;
    bool enableNetwork;
    std::string networkEndpoint;
};