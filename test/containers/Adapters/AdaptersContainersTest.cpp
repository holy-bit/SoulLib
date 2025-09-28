#include <gtest/gtest.h>
#include "containers/Adapters/SoulStack.h"
#include "containers/Adapters/SoulPriorityQueue.h"
#include <algorithm>

using namespace ContainerSystem::Adapters;

TEST(AdaptersContainers, StackBasicOperations) {
    SoulStack<int> stack;
    stack.insert(1);
    stack.insert(2);
    stack.insert(3);
    EXPECT_EQ(stack.size(), 3);
    EXPECT_EQ(stack.peek(), 3);
    auto all = stack.getAll();
    EXPECT_EQ(all, (std::vector<int>{1,2,3}));
    auto firstEven = stack.findFirst([](int x){ return x % 2 == 0; });
    EXPECT_TRUE(firstEven.has_value());
    EXPECT_EQ(firstEven.value(), 2);
    stack.remove(2);
    EXPECT_EQ(stack.size(), 2);
    stack.pop();
    EXPECT_EQ(stack.size(), 1);
    stack.push(5);
    EXPECT_EQ(stack.peek(), 5);
}

TEST(AdaptersContainers, PriorityQueueBasicOperations) {
    SoulPriorityQueue<int> pq;
    pq.insert(5);
    pq.insert(1);
    pq.insert(3);
    EXPECT_EQ(pq.size(), 3);
    EXPECT_EQ(pq.peek(), 5);
    auto all = pq.getAll();
    EXPECT_EQ(all.front(), 5);
    auto firstGreater = pq.findFirst([](int x){ return x > 2; });
    EXPECT_TRUE(firstGreater.has_value());
    pq.remove(5);
    EXPECT_EQ(pq.size(), 2);
    pq.pop();
    EXPECT_EQ(pq.size(), 1);
    pq.push(4);
    EXPECT_EQ(pq.peek(), 4);
}
