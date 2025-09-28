#pragma once
#include <chrono>
#include <mutex>
#include <cstdint>

class TimeService {
public:
    TimeService();
    uint64_t getCurrentTime() const;
    void syncWithServerTime(uint64_t serverTime);
    int64_t getDeltaTime() const;
    bool isSynced() const;
    void updateRealTime();

private:
    mutable std::mutex timeMutex;
    uint64_t serverTimeAtSync;
    std::chrono::steady_clock::time_point lastSyncLocalTime;
    bool synced;
};