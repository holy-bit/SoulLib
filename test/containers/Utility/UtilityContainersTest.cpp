#include <gtest/gtest.h>
#include <vector>
#include "containers/Core/ContainerMemoryOptimizer.h"

using namespace ContainerSystem::Core;

TEST(ContainerMemoryOptimizerTests, OptimizeVectorReducesCapacity) {
    std::vector<int> vec;
    vec.reserve(10);
    vec.push_back(1);
    vec.push_back(2);
    size_t beforeCap = vec.capacity();
    ContainerMemoryOptimizer::optimize(vec);
    size_t afterCap = vec.capacity();
    EXPECT_EQ(afterCap, vec.size());
    EXPECT_LT(afterCap, beforeCap);
}