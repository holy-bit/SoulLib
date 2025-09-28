#ifndef CONTAINERS_ASSOCIATIVE_SOULMULTISET_H
#define CONTAINERS_ASSOCIATIVE_SOULMULTISET_H

#include <concepts>
#include <optional>
#include <ranges>
#include <set>
#include <vector>
#include "containers/Core/IContainer.h"

namespace ContainerSystem::Associative {

/**
 * @brief Multiset adapter preserving duplicates while exposing the Soul container API.
 * @tparam T Value type stored in the multiset.
 */
template<typename T>
class SoulMultiset : public ContainerSystem::Core::IContainer<T> {
private:
    std::multiset<T> data;
public:
    SoulMultiset() = default;

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    explicit SoulMultiset(R&& range) {
        insert_range(std::forward<R>(range));
    }

    /// Adds a value, allowing duplicate entries.
    void insert(const T& value) override { data.insert(value); }

    /// Removes every instance of @p value from the container.
    void remove(const T& value) override { data.erase(value); }

    /// Materializes the multiset into a sorted vector snapshot.
    std::vector<T> getAll() const override {
        std::vector<T> result;
        result.reserve(data.size());
        for (const auto& value : data) {
            result.push_back(value);
        }
        return result;
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
        // multiset is ordered; no-op
    }

    /**
     * @brief Bulk-inserts elements from an input range, retaining duplicates.
     * @tparam R Range whose references convert to @tparam T.
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

#endif // CONTAINERS_ASSOCIATIVE_SOULMULTISET_H