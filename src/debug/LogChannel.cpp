#include "../../include/debug/LogChannel.h"

void LogChannel::SetLevel(LogLevel lvl) {
    level = lvl;
}

LogLevel LogChannel::GetLevel() const {
    return level;
}