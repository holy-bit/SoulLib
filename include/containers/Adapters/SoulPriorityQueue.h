#ifndef CONTAINERS_ADAPTERS_SOULPRIORITYQUEUE_H
#define CONTAINERS_ADAPTERS_SOULPRIORITYQUEUE_H

#include <queue>
#include <vector>
#include <algorithm>
#include <functional>
#include <optional>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"

namespace ContainerSystem::Adapters {

template<typename T, typename Compare = std::less<T>>
class SoulPriorityQueue : public ContainerSystem::Core::IContainer<T> {
private:
    std::priority_queue<T, std::vector<T>, Compare> data;
public:
    void insert(const T& value) override { data.push(value); }
    void remove(const T& value) override {
        auto vec = getAll();
        vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
        std::priority_queue<T, std::vector<T>, Compare> newPQ((Compare()));
        for (auto& v : vec) newPQ.push(v);
        data = std::move(newPQ);
    }
    std::vector<T> getAll() const override {
        std::vector<T> vec;
        auto temp = data;
        while (!temp.empty()) { vec.push_back(temp.top()); temp.pop(); }
        return vec;
    }
    size_t size() const override { return data.size(); }
    std::vector<T> findAll(typename ContainerSystem::Core::Predicate<T> pred) const override {
        auto vec = getAll();
        return ContainerSystem::Algorithms::Searcher::findAll(vec, pred);
    }
    std::optional<T> findFirst(typename ContainerSystem::Core::Predicate<T> pred) const override {
        auto vec = getAll();
        return ContainerSystem::Algorithms::Searcher::findFirst(vec, pred);
    }
    void sort(typename ContainerSystem::Core::Comparator<T> comp) override {
        auto vec = getAll();
        ContainerSystem::Algorithms::Sorter::sortByComparator(vec, comp);
        std::priority_queue<T, std::vector<T>, Compare> newPQ((Compare()));
        for (auto& v : vec) newPQ.push(v);
        data = std::move(newPQ);
    }
    // Additional priority queue operations
    T peek() const { return data.top(); }
    void pop() { data.pop(); }
    void push(const T& value) { insert(value); }
};

} // namespace ContainerSystem::Adapters

#endif // CONTAINERS_ADAPTERS_SOULPRIORITYQUEUE_H