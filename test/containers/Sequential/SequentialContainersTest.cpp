#include "containers/Sequential/SoulVector.h"
#include "containers/Sequential/SoulList.h"
#include "containers/Sequential/SoulForwardList.h"
#include "containers/Sequential/SoulDeque.h"
#include "containers/Sequential/SoulArray.h"
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"
#include "containers/Core/ContainerTags.h"
#include <algorithm>
#include <array>
#include <vector>
#include <gtest/gtest.h>

using namespace ContainerSystem::Sequential;

TEST(SequentialContainers, VectorBasicOperations) {
    SoulVector<int> vec;
    vec.insert(1);
    vec.insert(2);
    vec.insert(3);
    EXPECT_EQ(vec.size(), 3);
    vec.remove(2);
    auto all = vec.getAll();
    EXPECT_EQ(all.size(), 2);
    EXPECT_EQ(std::find(all.begin(), all.end(), 2), all.end());
}

TEST(SequentialContainers, ListBasicOperations) {
    SoulList<int> lst;
    lst.insert(10);
    lst.insert(20);
    EXPECT_EQ(lst.size(), 2);
    lst.remove(10);
    auto all = lst.getAll();
    EXPECT_EQ(all.size(), 1);
}

TEST(SequentialContainers, ForwardListBasicOperations) {
    SoulForwardList<int> fl;
    fl.insert(5);
    EXPECT_EQ(fl.size(), 1);
    fl.remove(5);
    EXPECT_TRUE(fl.getAll().empty());
}

TEST(SequentialContainers, DequeBasicOperations) {
    SoulDeque<int> dq;
    dq.insert(7);
    dq.insert(8);
    EXPECT_EQ(dq.size(), 2);
}

TEST(SequentialContainers, ArrayBasicOperations) {
    SoulArray<int, 5> arr;
    arr.insert(3);
    EXPECT_EQ(arr.size(), 1);
}

TEST(SequentialContainers, VectorSmallBufferOptimization) {
    Memory::Core::MemoryManager manager;
    Memory::Core::MemoryRegistry::Set(manager);

    using SmallSoulVector = SoulVector<int, 4>;
    SmallSoulVector vec;

    EXPECT_EQ(manager.getAllocationCount(), 0u);

    for (int i = 0; i < 4; ++i) {
        vec.insert(i);
    }

    EXPECT_EQ(vec.size(), 4u);
    EXPECT_EQ(manager.getAllocationCount(), 0u) << "SBO path should avoid heap allocations";

    vec.insert(99); // Forces transition to heap storage
    EXPECT_EQ(vec.size(), 5u);
    EXPECT_GT(manager.getAllocationCount(), 0u) << "Growth beyond SBO should allocate";

    vec.remove(99);
    vec.shrink_to_fit();

    EXPECT_EQ(vec.size(), 4u);
    EXPECT_EQ(manager.getAllocatedByTag(ContainerSystem::Core::VectorTag::value()), 0u)
        << "Shrink-to-fit should release heap storage when fitting in SBO";

    Memory::Core::MemoryRegistry::Reset();
}

TEST(SequentialContainers, VectorConstructFromRange) {
    std::array<int, 3> values {1, 2, 3};
    SoulVector<int> vec(values);

    ASSERT_EQ(vec.size(), values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        EXPECT_EQ(vec.begin()[i], values[i]);
    }
}

TEST(SequentialContainers, VectorAppendRangeExtendsSequence) {
    SoulVector<int> vec {1, 2};
    const std::vector<int> more {3, 4, 5};

    vec.append_range(more);

    ASSERT_EQ(vec.size(), 5u);
    EXPECT_EQ(vec.begin()[2], 3);
    EXPECT_EQ(vec.begin()[4], 5);
}