#ifndef CONTAINERS_UNORDERED_SOULUNORDEREDSET_H
#define CONTAINERS_UNORDERED_SOULUNORDEREDSET_H

#include <concepts>
#include <optional>
#include <ranges>
#include <unordered_set>
#include <vector>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"
#include "containers/Core/ContainerTags.h"
#include "Memory/Core/TaggedMemoryAllocator.h"

namespace ContainerSystem::Unordered {

/**
 * @brief Hash-based set adapter built atop std::unordered_set.
 * @tparam T Value type stored with unique keys.
 */
template<typename T>
class SoulUnorderedSet : public ContainerSystem::Core::IContainer<T> {
private:
    std::unordered_set<T, std::hash<T>, std::equal_to<T>,
        Memory::Core::TaggedMemoryAllocator<T, ContainerSystem::Core::UnorderedSetTag>> data;
public:
    SoulUnorderedSet() = default;

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    explicit SoulUnorderedSet(R&& range) {
        insert_range(std::forward<R>(range));
    }

    /// Inserts a value if it is not already present.
    void insert(const T& value) override { data.insert(value); }

    /// Removes the specified value from the set.
    void remove(const T& value) override { data.erase(value); }

    /// Produces an unordered snapshot of the set.
    std::vector<T> getAll() const override {
        // Changed to use standard vector with default allocator
        std::vector<T> result;
        result.reserve(data.size());
        result.insert(result.end(), data.begin(), data.end());
        return result;
    }
    size_t size() const override { return data.size(); }
    std::vector<T> findAll(typename ContainerSystem::Core::Predicate<T> pred) const override {
        // Changed to use standard vector with default allocator
        std::vector<T> result;
        for (const auto& item : data) {
            if (pred(item)) {
                result.push_back(item);
            }
        }
        return result;
    }
    std::optional<T> findFirst(typename ContainerSystem::Core::Predicate<T> pred) const override {
        for (const auto& item : data) {
            if (pred(item)) {
                return item;
            }
        }
        return std::nullopt;
    }
    void sort(typename ContainerSystem::Core::Comparator<T> comp) override {
        auto vec = getAll();
        ContainerSystem::Algorithms::Sorter::sortByComparator(vec, comp);
        data.clear(); 
        for (auto& v : vec) data.insert(v);
    }

    /**
     * @brief Inserts elements from an input range, ignoring duplicates.
     * @tparam R Range convertible to @tparam T.
     */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    void insert_range(R&& range) {
        for (auto&& value : range) {
            data.insert(static_cast<T>(value));
        }
    }
};

} // namespace ContainerSystem::Unordered

#endif // CONTAINERS_UNORDERED_SOULUNORDEREDSET_H