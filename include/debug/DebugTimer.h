#pragma once

#include <chrono>

class DebugTimer {
public:
    void Start();
    void Stop();
    std::chrono::duration<double> Elapsed() const;
    void Reset();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    bool running = false;
};