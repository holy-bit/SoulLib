#pragma once

#include <functional>
#include <string>
#include "LogEntry.h"
#include "SourceInfo.h"

class AssertHandler {
public:
    void Check(bool cond, const std::string& msg, const SourceInfo& info);
    void SetFailureCallback(std::function<void(const LogEntry&)> cb);

private:
    std::function<void(const LogEntry&)> failureCallback;
};