#include "../../include/debug/Formatter.h"

std::string Formatter::Format(const LogEntry& entry) {
    // Basic formatting implementation
    return pattern + ": " + entry.message;
}

void Formatter::SetPattern(const std::string& fmt) {
    pattern = fmt;
}