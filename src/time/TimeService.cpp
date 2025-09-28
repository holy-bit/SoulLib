#include "time/TimeService.h"
#include <chrono>
#include <mutex>

TimeService::TimeService()
    : serverTimeAtSync(0),
      lastSyncLocalTime(std::chrono::steady_clock::now()),
      synced(false) {}

uint64_t TimeService::getCurrentTime() const {
    std::lock_guard<std::mutex> lock(timeMutex);
    if (!synced) return 0;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSyncLocalTime).count();
    return serverTimeAtSync + static_cast<uint64_t>(elapsed);
}

void TimeService::syncWithServerTime(uint64_t serverTime) {
    std::lock_guard<std::mutex> lock(timeMutex);
    lastSyncLocalTime = std::chrono::steady_clock::now();
    serverTimeAtSync = serverTime;
    synced = true;
}

int64_t TimeService::getDeltaTime() const {
    std::lock_guard<std::mutex> lock(timeMutex);
    if (!synced) return 0;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSyncLocalTime).count();
    return elapsed;
}

bool TimeService::isSynced() const {
    std::lock_guard<std::mutex> lock(timeMutex);
    return synced;
}

void TimeService::updateRealTime() {
    std::lock_guard<std::mutex> lock(timeMutex);
    if (!synced) return;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastSyncLocalTime).count();
    serverTimeAtSync += static_cast<uint64_t>(elapsed);
    lastSyncLocalTime = now;
}