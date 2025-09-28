#ifndef CONTAINERS_SEQUENTIAL_SOULARRAY_H
#define CONTAINERS_SEQUENTIAL_SOULARRAY_H

#include <array>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <optional>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"

namespace ContainerSystem::Sequential {

template<typename T, std::size_t N>
class SoulArray : public ContainerSystem::Core::IContainer<T> {
private:
    std::array<T, N> data;
    std::size_t currentSize = 0;
public:
    void insert(const T& value) override {
        if (currentSize < N) {
            data[currentSize++] = value;
        }
    }
    
    void remove(const T& value) override {
        auto it = std::find(data.begin(), data.begin() + currentSize, value);
        if (it != data.begin() + currentSize) {
            // Shift elements to fill the gap
            std::copy(it + 1, data.begin() + currentSize, it);
            --currentSize;
        }
    }
    
    std::vector<T> getAll() const override {
        return std::vector<T>(data.begin(), data.begin() + currentSize);
    }
    
    size_t size() const override {
        return currentSize;
    }
    
    std::vector<T> findAll(typename ContainerSystem::Core::Predicate<T> pred) const override {
        std::vector<T> result;
        for (size_t i = 0; i < currentSize; ++i) {
            if (pred(data[i])) {
                result.push_back(data[i]);
            }
        }
        return result;
    }
    
    std::optional<T> findFirst(typename ContainerSystem::Core::Predicate<T> pred) const override {
        for (size_t i = 0; i < currentSize; ++i) {
            if (pred(data[i])) {
                return data[i];
            }
        }
        return std::nullopt;
    }
    
    void sort(typename ContainerSystem::Core::Comparator<T> comp) override {
        std::sort(data.begin(), data.begin() + currentSize, comp);
    }
};

} // namespace ContainerSystem::Sequential

#endif // CONTAINERS_SEQUENTIAL_SOULARRAY_H