#ifndef CONTAINERS_ASSOCIATIVE_SOULSET_H
#define CONTAINERS_ASSOCIATIVE_SOULSET_H

#include <concepts>
#include <optional>
#include <ranges>
#include <set>
#include <vector>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"

namespace ContainerSystem::Associative {

/**
 * @brief Ordered set adapter exposing the Soul container interface over std::set.
 *
 * @tparam T Value type stored in the set.
 */
template<typename T>
class SoulSet : public ContainerSystem::Core::IContainer<T> {
private:
    std::set<T> data;
public:
    SoulSet() = default;

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    explicit SoulSet(R&& range) {
        insert_range(std::forward<R>(range));
    }

    /// Inserts a value, discarding duplicates to preserve set guarantees.
    void insert(const T& value) override { data.insert(value); }

    /// Removes all instances of @p value from the set.
    void remove(const T& value) override { data.erase(value); }

    /// Returns a sorted snapshot of the underlying container.
    std::vector<T> getAll() const override {
        return std::vector<T>(data.begin(), data.end());
    }
    size_t size() const override { return data.size(); }
    std::vector<T> findAll(typename ContainerSystem::Core::Predicate<T> pred) const override {
        std::vector<T> result;
        for (const auto& v : data) if (pred(v)) result.push_back(v);
        return result;
    }
    std::optional<T> findFirst(typename ContainerSystem::Core::Predicate<T> pred) const override {
        for (const auto& v : data) if (pred(v)) return v;
        return std::nullopt;
    }
    void sort(typename ContainerSystem::Core::Comparator<T> comp) override {
        // set is already sorted; no-op or reordering via vector if needed
    }

    /**
     * @brief Inserts elements from an input range while keeping unique keys.
     * @tparam R Range producing values convertible to @tparam T.
     */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    void insert_range(R&& range) {
        for (auto&& value : range) {
            data.insert(static_cast<T>(value));
        }
    }
};

} // namespace ContainerSystem::Associative

#endif // CONTAINERS_ASSOCIATIVE_SOULSET_H