#pragma once

#include <chrono>

struct Timestamp {
    std::chrono::time_point<std::chrono::system_clock> time;

    static Timestamp Now() {
        return Timestamp{std::chrono::system_clock::now()};
    }
};