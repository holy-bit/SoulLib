#include <gtest/gtest.h>
#include "containers/Unordered/SoulUnorderedSet.h"
#include "containers/Unordered/SoulUnorderedMap.h"
#include <array>
#include <string>
#include <vector>

using namespace ContainerSystem::Unordered;

TEST(UnorderedContainers, UnorderedSetBasic) {
    SoulUnorderedSet<int> uset;
    uset.insert(1);
    uset.insert(2);
    uset.insert(3);
    EXPECT_EQ(uset.size(), 3);
    auto all = uset.getAll();
    EXPECT_EQ(all.size(), 3);
    auto evens = uset.findAll([](int x){ return x % 2 == 0; });
    EXPECT_EQ(evens.size(), 1);
    auto first = uset.findFirst([](int x){ return x > 1; });
    EXPECT_TRUE(first.has_value());
    uset.remove(2);
    EXPECT_EQ(uset.size(), 2);
}

TEST(UnorderedContainers, UnorderedSetConstructFromRange) {
    std::vector<int> values {1, 2, 2, 3};
    SoulUnorderedSet<int> uset(values);

    EXPECT_EQ(uset.size(), 3u);
    auto all = uset.getAll();
    for (int value : values) {
        EXPECT_TRUE(uset.findFirst([value](int v){ return v == value; }).has_value());
    }
}

TEST(UnorderedContainers, UnorderedSetInsertRange) {
    SoulUnorderedSet<int> uset;
    uset.insert(10);

    const std::array<int, 3> more {10, 11, 12};
    uset.insert_range(more);

    EXPECT_EQ(uset.size(), 3u);
    EXPECT_TRUE(uset.findFirst([](int v){ return v == 11; }).has_value());
}

TEST(UnorderedContainers, UnorderedMapBasic) {
    SoulUnorderedMap<std::string, int> umap;
    umap.insert({"a", 1});
    umap.insert({"b", 2});
    EXPECT_EQ(umap.size(), 2);
    auto all = umap.getAll();
    EXPECT_EQ(all.size(), 2);
    auto filtered = umap.findAll([](const auto& kv){ return kv.second > 1; });
    EXPECT_EQ(filtered.size(), 1);
    auto first = umap.findFirst([](const auto& kv){ return kv.first == "a"; });
    EXPECT_TRUE(first.has_value());
    umap.remove({"a", 0});
    EXPECT_EQ(umap.size(), 1);
}

TEST(UnorderedContainers, UnorderedMapConstructFromRange) {
    std::vector<std::pair<std::string, int>> entries { {"k1", 1}, {"k2", 2}, {"k1", 3} };
    SoulUnorderedMap<std::string, int> umap(entries);

    EXPECT_EQ(umap.size(), 2u);
    EXPECT_EQ(umap.findFirst([](const auto& kv){ return kv.first == "k1"; })->second, 3);
}

TEST(UnorderedContainers, UnorderedMapInsertRangeOverwrites) {
    SoulUnorderedMap<std::string, int> umap;
    umap.insert({"alpha", 1});

    const std::array<std::pair<std::string, int>, 2> more {{{"beta", 2}, {"alpha", 9}}};
    umap.insert_range(more);

    EXPECT_EQ(umap.size(), 2u);
    EXPECT_EQ(umap.findFirst([](const auto& kv){ return kv.first == "alpha"; })->second, 9);
}
