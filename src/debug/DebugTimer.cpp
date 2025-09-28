#include "../../include/debug/DebugTimer.h"

void DebugTimer::Start() {
    startTime = std::chrono::high_resolution_clock::now();
    running = true;
}

void DebugTimer::Stop() {
    endTime = std::chrono::high_resolution_clock::now();
    running = false;
}

std::chrono::duration<double> DebugTimer::Elapsed() const {
    if (running) {
        return std::chrono::high_resolution_clock::now() - startTime;
    } else {
        return endTime - startTime;
    }
}

void DebugTimer::Reset() {
    running = false;
    startTime = std::chrono::high_resolution_clock::now();
    endTime = startTime;
}