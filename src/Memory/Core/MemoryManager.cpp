#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include "debug/Debug.h"
#include "Memory/Core/MemoryManager.h"

namespace Memory::Core {

void MemoryManager::setDebugMode(bool enabled) {
    debugMode_ = enabled;
}

void MemoryManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    allocations_.clear();
}

namespace {
#ifdef SOUL_DEBUG
std::string formatTag(const MemoryTag& tag) {
    if (!tag.label.empty()) {
        return std::string(tag.label);
    }
    return std::to_string(tag.hash);
}
#else
std::string formatTag(const MemoryTag& tag) {
    return std::to_string(tag.hash);
}
#endif
} // namespace

void MemoryManager::registerAllocation(void* ptr, size_t size, MemoryTag tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    allocations_[ptr] = {size, tag};
    if (debugMode_) {
        std::stringstream ss;
        ss << "Allocated " << size << " bytes at " << ptr << " [" << formatTag(tag) << "]";
        Debug::Debug(ss.str(), {__FILE__, __LINE__, __FUNCTION__});
    }
}

void MemoryManager::unregisterAllocation(void* ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = allocations_.find(ptr);
    if (it != allocations_.end()) {
        if (debugMode_) {
            std::stringstream ss;
            ss << "Deallocated " << it->second.size << " bytes at " << ptr << " [" << formatTag(it->second.tag) << "]";
            Debug::Debug(ss.str(), {__FILE__, __LINE__, __FUNCTION__});
        }
        allocations_.erase(it);
    }
}

void MemoryManager::reportLeaks() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& entry : allocations_) {
        const auto& ptr = entry.first;
        const auto& info = entry.second;
        std::stringstream ss;
        ss << "Leaked " << info.size << " bytes at " << ptr << " [" << formatTag(info.tag) << "]";
        Debug::Debug(ss.str(), {__FILE__, __LINE__, __FUNCTION__});
    }
}

size_t MemoryManager::getTotalAllocated() {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = 0;
    for (const auto& [_, info] : allocations_) total += info.size;
    return total;
}

size_t MemoryManager::getAllocationCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return allocations_.size();
}

size_t MemoryManager::getAllocationSize(void* ptr) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = allocations_.find(ptr);
    return (it != allocations_.end()) ? it->second.size : 0;
}

size_t MemoryManager::getAllocatedByTag(MemoryTag tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t sum = 0;
    for (const auto& entry : allocations_) {
        if (entry.second.tag == tag) {
            sum += entry.second.size;
        }
    }
    return sum;
}

MemoryStatistics MemoryManager::snapshot() {
    std::lock_guard<std::mutex> lock(mutex_);
    MemoryStatistics stats;
    stats.totalBytes = 0;
    stats.allocationCount = allocations_.size();

    std::unordered_map<uint32_t, MemoryTagStats> perTag;
    perTag.reserve(allocations_.size());

    for (const auto& [pointer, info] : allocations_) {
        stats.totalBytes += info.size;
        auto& bucket = perTag[info.tag.hash];
        bucket.tag = info.tag;
        bucket.bytes += info.size;
        bucket.allocations += 1;
    }

    stats.tags.reserve(perTag.size());
    for (auto& [_, tagStats] : perTag) {
        stats.tags.emplace_back(std::move(tagStats));
    }
    return stats;
}

} // namespace Memory::Core
