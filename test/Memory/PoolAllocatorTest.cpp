#include <gtest/gtest.h>
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"
#include "Memory/Core/PoolAllocator.h"

using namespace Memory::Core;

struct PoolAllocatorTest : public ::testing::Test {
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

TEST_F(PoolAllocatorTest, AllocateAndRecycle) {
    PoolAllocator<int, 4> pool;
    EXPECT_EQ(pool.capacity(), 4u);
    EXPECT_EQ(pool.available(), 4u);

    int* a = pool.allocate();
    int* b = pool.allocate();
    int* c = pool.allocate();
    int* d = pool.allocate();

    EXPECT_NE(a, nullptr);
    EXPECT_NE(b, nullptr);
    EXPECT_NE(c, nullptr);
    EXPECT_NE(d, nullptr);
    EXPECT_EQ(pool.available(), 0u);

    pool.deallocate(b);
    pool.deallocate(d);
    EXPECT_EQ(pool.available(), 2u);

    int* e = pool.allocate();
    EXPECT_NE(e, nullptr);
    EXPECT_EQ(pool.available(), 1u);
}

TEST_F(PoolAllocatorTest, ExhaustionReturnsNull) {
    PoolAllocator<int, 2> pool;
    (void)pool.allocate();
    (void)pool.allocate();
    EXPECT_EQ(pool.allocate(), nullptr);
}

TEST_F(PoolAllocatorTest, ResetRestoresCapacity) {
    PoolAllocator<int, 3> pool;
    pool.allocate();
    pool.allocate();
    EXPECT_EQ(pool.available(), 1u);
    pool.reset();
    EXPECT_EQ(pool.available(), 3u);
}
