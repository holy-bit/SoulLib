#include <gtest/gtest.h>
#include <cstdint>
#include "Memory/Core/ArenaAllocator.h"
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"

using namespace Memory::Core;

struct ArenaAllocatorTest : public ::testing::Test {
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

TEST_F(ArenaAllocatorTest, AllocateSequentially) {
    ArenaAllocator arena(manager, 256);
    void* first = arena.allocate(64);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(arena.used(), 64u);

    void* second = arena.allocate(32, alignof(double));
    ASSERT_NE(second, nullptr);
    EXPECT_GT(reinterpret_cast<std::uintptr_t>(second), reinterpret_cast<std::uintptr_t>(first));
    EXPECT_GE(arena.used(), 96u);
}

TEST_F(ArenaAllocatorTest, CreateConstructsObjects) {
    struct Sample {
        int a;
        double b;
    };

    ArenaAllocator arena(manager, 128);
    Sample* instance = arena.create<Sample>(Sample{42, 3.14});
    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(instance->a, 42);
    EXPECT_DOUBLE_EQ(instance->b, 3.14);
}

TEST_F(ArenaAllocatorTest, ResetAllowsReuse) {
    ArenaAllocator arena(manager, 64);
    (void)arena.allocate(32);
    EXPECT_LT(arena.remaining(), arena.capacity());
    arena.reset();
    EXPECT_EQ(arena.used(), 0u);
    EXPECT_EQ(arena.remaining(), arena.capacity());
}

TEST_F(ArenaAllocatorTest, ThrowsWhenOutOfMemory) {
    ArenaAllocator arena(manager, 32);
    EXPECT_NO_THROW(arena.allocate(16));
    EXPECT_THROW(arena.allocate(32), std::bad_alloc);
}
