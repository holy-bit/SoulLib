#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <gtest/gtest.h>
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"
#include "containers/Core/IContainer.h"

using namespace ContainerSystem::Algorithms;
using namespace ContainerSystem::Core;

TEST(SorterTests, SortAscending) {
    std::vector<int> v = {3, 1, 2};
    Sorter::sortAscending(v);
    EXPECT_EQ(v, (std::vector<int>{1, 2, 3}));
}

TEST(SorterTests, SortDescending) {
    std::vector<int> v = {1, 3, 2};
    Sorter::sortDescending(v);
    EXPECT_EQ(v, (std::vector<int>{3, 2, 1}));
}

TEST(SorterTests, SortByComparator) {
    std::vector<int> v = {4, 2, 5, 1};
    // Explicitly create a std::function to match the expected type
    std::function<bool(const int&, const int&)> comp = [](const int& a, const int& b) { 
        return a > b; 
    };
    Sorter::sortByComparator(v, comp);
    EXPECT_EQ(v, (std::vector<int>{5, 4, 2, 1}));
}

TEST(SorterTests, StableSort) {
    std::vector<std::pair<int, char>> v = {{2, 'a'}, {1, 'b'}, {2, 'c'}};
    // Explicitly create a std::function to match the expected type
    std::function<bool(const std::pair<int, char>&, const std::pair<int, char>&)> comp = 
        [](const std::pair<int, char>& a, const std::pair<int, char>& b) { 
            return a.first < b.first; 
        };
    Sorter::stableSort(v, comp);
    EXPECT_EQ(v[0].second, 'b');
    EXPECT_EQ(v[1].second, 'a');
    EXPECT_EQ(v[2].second, 'c');
}

TEST(SearcherTests, FindFirst) {
    std::vector<int> v = {1, 2, 3, 4};
    // Explicitly create a std::function to match the expected type
    std::function<bool(const int&)> pred = [](const int& x) { 
        return x % 2 == 0; 
    };
    auto res = Searcher::findFirst(v, pred);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 2);
}

TEST(SearcherTests, FindAll) {
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    // Explicitly create a std::function to match the expected type
    std::function<bool(const int&)> pred = [](const int& x) { 
        return x > 3; 
    };
    auto all = Searcher::findAll(v, pred);
    EXPECT_EQ(all, (std::vector<int>{4, 5, 6}));
}

TEST(SearcherTests, IndexOf) {
    std::vector<int> v = {10, 20, 30};
    auto idx = Searcher::indexOf(v, 20);
    EXPECT_TRUE(idx.has_value());
    EXPECT_EQ(idx.value(), 1);
}

TEST(SearcherTests, BinarySearchFound) {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto idx = Searcher::binarySearch(v, 4);
    EXPECT_TRUE(idx.has_value());
    EXPECT_EQ(idx.value(), 3);
}

TEST(SearcherTests, BinarySearchNotFound) {
    std::vector<int> v = {1, 3, 5};
    auto idx = Searcher::binarySearch(v, 2);
    EXPECT_FALSE(idx.has_value());
}