#ifndef CONTAINERS_ALGORITHMS_SEARCHER_H
#define CONTAINERS_ALGORITHMS_SEARCHER_H

#include <vector>
#include <optional>
#include <algorithm>
#include "containers/Core/IContainer.h"

namespace ContainerSystem::Algorithms {

class Searcher {
public:
    template<typename T>
    static std::optional<T> findFirst(const std::vector<T>& vec, const Core::Predicate<T>& pred) {
        auto it = std::find_if(vec.begin(), vec.end(), pred);
        if (it != vec.end()) return *it;
        return std::nullopt;
    }

    template<typename T>
    static std::vector<T> findAll(const std::vector<T>& vec, const Core::Predicate<T>& pred) {
        std::vector<T> result;
        for (const auto& v : vec) if (pred(v)) result.push_back(v);
        return result;
    }

    template<typename T>
    static std::optional<size_t> indexOf(const std::vector<T>& vec, const T& value) {
        auto it = std::find(vec.begin(), vec.end(), value);
        if (it != vec.end()) return static_cast<size_t>(std::distance(vec.begin(), it));
        return std::nullopt;
    }

    template<typename T>
    static std::optional<size_t> binarySearch(const std::vector<T>& vec, const T& value) {
        auto it = std::lower_bound(vec.begin(), vec.end(), value);
        if (it != vec.end() && *it == value) return static_cast<size_t>(std::distance(vec.begin(), it));
        return std::nullopt;
    }
};

} // namespace ContainerSystem::Algorithms

#endif // CONTAINERS_ALGORITHMS_SEARCHER_H