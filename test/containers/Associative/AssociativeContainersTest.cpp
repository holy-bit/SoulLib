#include <gtest/gtest.h>
#include "containers/Associative/SoulSet.h"
#include "containers/Associative/SoulMultiset.h"
#include "containers/Associative/SoulMultimap.h"
#include "containers/Associative/SoulMap.h"
#include "containers/Associative/SoulFlatMap.h"
#include <algorithm>
#include <array>
#include <vector>

using namespace ContainerSystem::Associative;

TEST(AssociativeContainers, SetBasic) {
    SoulSet<int> s;
    s.insert(3);
    s.insert(1);
    s.insert(2);
    EXPECT_EQ(s.size(), 3);
    auto all = s.getAll();
    EXPECT_EQ(all, (std::vector<int>{1,2,3}));
    auto evens = s.findAll([](int x){ return x % 2 == 0; });
    EXPECT_EQ(evens, (std::vector<int>{2}));
    auto first = s.findFirst([](int x){ return x > 1; });
    EXPECT_TRUE(first.has_value());
    s.remove(2);
    EXPECT_EQ(s.size(), 2);
}

TEST(AssociativeContainers, MultisetBasic) {
    SoulMultiset<int> ms;
    ms.insert(5);
    ms.insert(5);
    ms.insert(3);
    EXPECT_EQ(ms.size(), 3);
    auto all = ms.getAll();
    EXPECT_EQ(all, (std::vector<int>{3,5,5}));
    ms.remove(5);
    EXPECT_EQ(ms.size(), 1); // removes all equal elements
}

TEST(AssociativeContainers, MultisetConstructFromRange) {
    std::vector<int> values {2, 2, 3, 1};
    SoulMultiset<int> ms(values);

    EXPECT_EQ(ms.size(), 4u);
    auto all = ms.getAll();
    EXPECT_EQ(all.front(), 1);
    EXPECT_EQ(all.back(), 3);
    std::size_t duplicates = 0;
    for (int value : all) {
        if (value == 2) {
            ++duplicates;
        }
    }
    EXPECT_EQ(duplicates, 2u);
}

TEST(AssociativeContainers, MultisetInsertRangePreservesDuplicates) {
    SoulMultiset<int> ms;
    ms.insert(7);

    const std::array<int, 3> more {7, 8, 7};
    ms.insert_range(more);

    EXPECT_EQ(ms.size(), 4u);
    const auto duplicates = ms.findAll([](int value) { return value == 7; });
    EXPECT_EQ(duplicates.size(), 3u);
}

TEST(AssociativeContainers, MapBasic) {
    SoulMap<std::string,int> m;
    m.insert({"one",1});
    m.insert({"two",2});
    EXPECT_EQ(m.size(), 2);
    auto all = m.getAll();
    EXPECT_EQ(all.size(), 2);
    auto found = m.findFirst([](const auto& kv){ return kv.second==2; });
    EXPECT_TRUE(found.has_value());
    m.remove({"one",0});
    EXPECT_EQ(m.size(), 1);
}

TEST(AssociativeContainers, FlatMapBasicOperations) {
    SoulFlatMap<std::string, int> flatMap;
    flatMap.insert({"alpha", 1});
    flatMap.insert({"beta", 2});
    flatMap.insert({"beta", 3}); // overwrite existing key

    EXPECT_EQ(flatMap.size(), 2u);
    EXPECT_TRUE(flatMap.contains("beta"));

    auto betaValue = flatMap.lookup("beta");
    ASSERT_TRUE(betaValue.has_value());
    EXPECT_EQ(betaValue.value(), 3);

    flatMap.insert({"gamma", 4});
    flatMap.remove({"alpha", 0});
    EXPECT_EQ(flatMap.size(), 2u);

    auto all = flatMap.getAll();
    EXPECT_EQ(all.front().first, "beta");

    int& deltaRef = flatMap["delta"];
    EXPECT_EQ(flatMap.size(), 3u);
    EXPECT_EQ(deltaRef, 0);
    deltaRef = 42;
    EXPECT_EQ(flatMap.lookup("delta").value(), 42);
}

TEST(AssociativeContainers, FlatMapConstructFromRange) {
    std::vector<std::pair<std::string, int>> entries {{"beta", 2}, {"alpha", 1}, {"alpha", 5}};
    SoulFlatMap<std::string, int> flatMap(entries);

    EXPECT_EQ(flatMap.size(), 2u);
    auto all = flatMap.getAll();
    ASSERT_EQ(all.front().first, "alpha");
    EXPECT_EQ(flatMap.lookup("alpha").value(), 5);
}

TEST(AssociativeContainers, FlatMapInsertRangeMergesKeys) {
    SoulFlatMap<std::string, int> flatMap;
    flatMap.insert({"gamma", 3});

    const std::array<std::pair<std::string, int>, 2> more {{{"beta", 2}, {"epsilon", 5}}};
    flatMap.insert_range(more);

    EXPECT_EQ(flatMap.size(), 3u);
    EXPECT_TRUE(flatMap.contains("beta"));
    EXPECT_EQ(flatMap.lookup("epsilon").value(), 5);
}

TEST(AssociativeContainers, SetConstructFromRange) {
    std::array<int, 5> values {4, 2, 2, 1, 3};
    SoulSet<int> s(values);

    EXPECT_EQ(s.size(), 4u);
    auto all = s.getAll();
    EXPECT_EQ(all.front(), 1);
    EXPECT_EQ(all.back(), 4);
}

TEST(AssociativeContainers, SetInsertRangeDeduplicates) {
    SoulSet<int> s;
    s.insert(5);

    const std::vector<int> more {5, 6, 7};
    s.insert_range(more);

    EXPECT_EQ(s.size(), 3u);
    EXPECT_TRUE(s.findFirst([](int value) { return value == 6; }).has_value());
}

TEST(AssociativeContainers, MapConstructFromRange) {
    std::vector<std::pair<std::string, int>> entries {{"one", 1}, {"two", 2}, {"one", 5}};
    SoulMap<std::string, int> m(entries);

    EXPECT_EQ(m.size(), 2u);
    auto found = m.findFirst([](const auto& kv) { return kv.first == "one"; });
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->second, 5);
}

TEST(AssociativeContainers, MapInsertRangeOverwritesExisting) {
    SoulMap<std::string, int> m;
    m.insert({"alpha", 1});

    const std::array<std::pair<std::string, int>, 2> more {{{"beta", 2}, {"alpha", 7}}};
    m.insert_range(more);

    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m.findFirst([](const auto& kv) { return kv.first == "alpha"; })->second, 7);
}

TEST(AssociativeContainers, MultimapConstructFromRange) {
    std::vector<std::pair<std::string, int>> entries { {"a", 1}, {"a", 2}, {"b", 3} };
    SoulMultimap<std::string, int> mmap(entries);

    EXPECT_EQ(mmap.size(), 3u);
    auto all = mmap.getAll();
    std::size_t countA = 0;
    for (const auto& kv : all) {
        if (kv.first == "a") {
            ++countA;
        }
    }
    EXPECT_EQ(countA, 2u);
}

TEST(AssociativeContainers, MultimapInsertRangeAddsDuplicates) {
    SoulMultimap<std::string, int> mmap;
    mmap.insert({"a", 1});

    const std::array<std::pair<std::string, int>, 2> more {{{"a", 4}, {"b", 5}}};
    mmap.insert_range(more);

    EXPECT_EQ(mmap.size(), 3u);
    auto all = mmap.getAll();
    std::size_t countDuplicates = 0;
    for (const auto& kv : all) {
        if (kv.first == "a") {
            ++countDuplicates;
        }
    }
    EXPECT_EQ(countDuplicates, 2u);
    EXPECT_TRUE(mmap.findFirst([](const auto& kv){ return kv.second == 5; }).has_value());
}