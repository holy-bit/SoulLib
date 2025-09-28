#include <gtest/gtest.h>
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"
#include "Memory/Core/MemoryTag.h"
#include "Memory/Profiler/MemoryProfiler.h"

using namespace Memory::Profiler;
using namespace Memory::Core;

struct MemoryProfilerTest : public ::testing::Test {
    MemoryManager manager;

    void SetUp() override {
        MemoryRegistry::Set(manager);
        manager.clear();
        manager.setDebugMode(false);
    }

    void TearDown() override {
        manager.clear();
        MemoryRegistry::Reset();
    }
};

TEST_F(MemoryProfilerTest, MeasureRawReturnsCorrectSize) {
    void* ptr = reinterpret_cast<void*>(0x5000);
    size_t size = 256;
    manager.registerAllocation(ptr, size, SOUL_MEMORY_TAG("profilerTag"));
    EXPECT_EQ(MemoryProfiler::measureRaw(ptr), size);
}

TEST_F(MemoryProfilerTest, MeasureRawUnregisteredPtrReturnsZero) {
    void* ptr = reinterpret_cast<void*>(0x6000);
    EXPECT_EQ(MemoryProfiler::measureRaw(ptr), 0u);
}
