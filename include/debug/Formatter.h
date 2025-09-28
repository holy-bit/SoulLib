#pragma once

#include <string>
#include "LogEntry.h"

class Formatter {
public:
    std::string Format(const LogEntry& entry);
    void SetPattern(const std::string& fmt);

private:
    std::string pattern;
};