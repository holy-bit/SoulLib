#ifndef CONTAINERS_ADAPTERS_SOULSTACK_H
#define CONTAINERS_ADAPTERS_SOULSTACK_H

#include <stack>
#include <vector>
#include <algorithm>
#include <functional>
#include <optional>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"

namespace ContainerSystem::Adapters {

template<typename T>
class SoulStack : public ContainerSystem::Core::IContainer<T> {
private:
    std::stack<T> data;
public:
    void insert(const T& value) override { data.push(value); }
    void remove(const T& value) override {
        auto vec = getAll();
        vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
        std::stack<T> newStack;
        for (auto& v : vec) newStack.push(v);
        data = std::move(newStack);
    }
    std::vector<T> getAll() const override {
        std::vector<T> vec;
        std::stack<T> temp = data;
        while (!temp.empty()) { vec.push_back(temp.top()); temp.pop(); }
        std::reverse(vec.begin(), vec.end());
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
        std::stack<T> newStack;
        for (auto& v : vec) newStack.push(v);
        data = std::move(newStack);
    }
    // Additional stack operations
    T peek() const { return data.top(); }
    void pop() { data.pop(); }
    void push(const T& value) { insert(value); }
};

} // namespace ContainerSystem::Adapters

#endif // CONTAINERS_ADAPTERS_SOULSTACK_H