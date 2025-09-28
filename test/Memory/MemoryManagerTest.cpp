#include <gtest/gtest.h>
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"
#include "Memory/Core/MemoryTag.h"

using namespace Memory::Core;

struct MemoryManagerTest : public ::testing::Test {
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

TEST_F(MemoryManagerTest, InitialState) {
    EXPECT_EQ(manager.getTotalAllocated(), 0u);
    EXPECT_EQ(manager.getAllocationCount(), 0u);
}

TEST_F(MemoryManagerTest, RegisterAndQuery) {
    void* ptr1 = reinterpret_cast<void*>(0x1000);
    void* ptr2 = reinterpret_cast<void*>(0x2000);
    manager.registerAllocation(ptr1, 16, SOUL_MEMORY_TAG("tagA"));
    manager.registerAllocation(ptr2, 32, SOUL_MEMORY_TAG("tagB"));

    EXPECT_EQ(manager.getAllocationCount(), 2u);
    EXPECT_EQ(manager.getTotalAllocated(), 48u);
    EXPECT_EQ(manager.getAllocationSize(ptr1), 16u);
    EXPECT_EQ(manager.getAllocationSize(ptr2), 32u);
    EXPECT_EQ(manager.getAllocatedByTag(SOUL_MEMORY_TAG("tagA")), 16u);
    EXPECT_EQ(manager.getAllocatedByTag(SOUL_MEMORY_TAG("tagB")), 32u);
}

TEST_F(MemoryManagerTest, UnregisterAllocation) {
    void* ptr = reinterpret_cast<void*>(0x3000);
    manager.registerAllocation(ptr, 64, SOUL_MEMORY_TAG("tagC"));
    EXPECT_EQ(manager.getAllocationCount(), 1u);
    manager.unregisterAllocation(ptr);
    EXPECT_EQ(manager.getAllocationCount(), 0u);
    EXPECT_EQ(manager.getTotalAllocated(), 0u);
}

TEST_F(MemoryManagerTest, ClearAll) {
    void* ptr = reinterpret_cast<void*>(0x4000);
    manager.registerAllocation(ptr, 128, SOUL_MEMORY_TAG("tagD"));
    EXPECT_EQ(manager.getAllocationCount(), 1u);
    manager.clear();
    EXPECT_EQ(manager.getAllocationCount(), 0u);
    EXPECT_EQ(manager.getTotalAllocated(), 0u);
}
