#include <gtest/gtest.h>
#include <memory>
#include "containers/Core/ContainerManager.h"
#include "containers/Sequential/SoulVector.h"

using namespace ContainerSystem::Core;
using namespace ContainerSystem::Sequential;

TEST(ContainerManagerTests, RegisterAndRetrieveContainer) {
    auto& mgr = ContainerManager::instance();
    auto vec = std::make_shared<SoulVector<int>>();
    mgr.registerContainer<int>("myvec", vec);
    auto retrieved = mgr.getContainer<int>("myvec");
    EXPECT_TRUE(static_cast<bool>(retrieved));
    EXPECT_EQ(retrieved.get(), vec.get());
    retrieved->insert(100);
    EXPECT_EQ(retrieved->size(), 1);
}

TEST(ContainerManagerTests, RetrieveNonExistentContainer) {
    auto& mgr = ContainerManager::instance();
    auto missing = mgr.getContainer<int>("does_not_exist");
    EXPECT_FALSE(static_cast<bool>(missing));
}