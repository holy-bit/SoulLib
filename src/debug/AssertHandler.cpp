#include "debug/AssertHandler.h"

void AssertHandler::Check(bool cond, const std::string& msg, const SourceInfo& info) {
    if (!cond && failureCallback) {
        LogEntry entry{LogLevel::Error, msg, info, {}}; // Timestamp to be filled appropriately
        failureCallback(entry);
    }
}

void AssertHandler::SetFailureCallback(std::function<void(const LogEntry&)> cb) {
    failureCallback = std::move(cb);
}