#ifndef CONTAINERS_UNORDERED_SOULUNORDEREDMAP_H
#define CONTAINERS_UNORDERED_SOULUNORDEREDMAP_H

#include <concepts>
#include <optional>
#include <ranges>
#include <unordered_map>
#include <vector>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"
#include "containers/Core/ContainerTags.h"
#include "Memory/Core/TaggedMemoryAllocator.h"

namespace ContainerSystem::Unordered {

/**
 * @brief Hash-map adapter with Soul container semantics built on std::unordered_map.
 * @tparam K Key type.
 * @tparam V Value type.
 */
template<typename K, typename V>
class SoulUnorderedMap : public ContainerSystem::Core::IContainer<std::pair<K, V>> {
private:
    std::unordered_map<K, V, std::hash<K>, std::equal_to<K>,
        Memory::Core::TaggedMemoryAllocator<std::pair<const K, V>, ContainerSystem::Core::UnorderedMapTag>> data;
public:
    SoulUnorderedMap() = default;

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, std::pair<K, V>>
    explicit SoulUnorderedMap(R&& range) {
        insert_range(std::forward<R>(range));
    }

    /// Inserts or overwrites the mapping for the specified key.
    void insert(const std::pair<K, V>& kv) override { data[kv.first] = kv.second; }

    /// Removes the entry identified by the key in @p kv.
    void remove(const std::pair<K, V>& kv) override { data.erase(kv.first); }

    /// Returns an unordered snapshot of all key-value pairs.
    std::vector<std::pair<K, V>> getAll() const override {
        std::vector<std::pair<K, V>> result;
        result.reserve(data.size());
        for (const auto& kv : data) result.push_back(kv);
        return result;
    }
    size_t size() const override { return data.size(); }
    std::vector<std::pair<K, V>> findAll(typename ContainerSystem::Core::Predicate<std::pair<K, V>> pred) const override {
        std::vector<std::pair<K, V>> result;
        for (const auto& item : data) {
            if (pred(item)) {
                result.push_back(item);
            }
        }
        return result;
    }
    std::optional<std::pair<K, V>> findFirst(typename ContainerSystem::Core::Predicate<std::pair<K, V>> pred) const override {
        for (const auto& item : data) {
            if (pred(item)) {
                return item;
            }
        }
        return std::nullopt;
    }
    void sort(typename ContainerSystem::Core::Comparator<std::pair<K, V>> comp) override {
        auto vec = getAll();
        ContainerSystem::Algorithms::Sorter::sortByComparator(vec, comp);
        data.clear();
        for (const auto& kv : vec) data[kv.first] = kv.second;
    }

    /**
     * @brief Inserts or updates entries from an input range.
     * @tparam R Range convertible to std::pair<K, V>.
     */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, std::pair<K, V>>
    void insert_range(R&& range) {
        for (auto&& entry : range) {
            std::pair<K, V> kv = static_cast<std::pair<K, V>>(entry);
            data[kv.first] = kv.second;
        }
    }
};

} // namespace ContainerSystem::Unordered

#endif // CONTAINERS_UNORDERED_SOULUNORDEREDMAP_H