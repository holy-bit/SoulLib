#ifndef CONTAINERS_ASSOCIATIVE_SOULMULTIMAP_H
#define CONTAINERS_ASSOCIATIVE_SOULMULTIMAP_H

#include <concepts>
#include <map>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>
#include "containers/Core/IContainer.h"

namespace ContainerSystem::Associative {

/**
 * @brief Ordered multimap adapter retaining duplicate keys while exposing the generic container API.
 *
 * @tparam K Key type, ordered according to std::multimap semantics.
 * @tparam V Value type associated with each key.
 */
template<typename K, typename V>
class SoulMultimap : public ContainerSystem::Core::IContainer<std::pair<K, V>> {
private:
    std::multimap<K, V> data;
public:
    SoulMultimap() = default;

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, std::pair<K, V>>
    explicit SoulMultimap(R&& range) {
        insert_range(std::forward<R>(range));
    }

    /// Inserts a key-value pair without removing existing entries sharing the same key.
    void insert(const std::pair<K, V>& kv) override {
        data.emplace(kv.first, kv.second);
    }
    /// Removes the first occurrence matching @p kv.
    void remove(const std::pair<K, V>& kv) override {
        auto range = data.equal_range(kv.first);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == kv.second) { data.erase(it); break; }
        }
    }
    /// Returns an ordered snapshot containing all stored pairs.
    std::vector<std::pair<K, V>> getAll() const override {
        std::vector<std::pair<K, V>> result;
        for (const auto& kv : data) result.push_back(kv);
        return result;
    }
    size_t size() const override { return data.size(); }
    std::vector<std::pair<K, V>> findAll(typename ContainerSystem::Core::Predicate<std::pair<K, V>> pred) const override {
        std::vector<std::pair<K, V>> result;
        for (const auto& kv : getAll()) if (pred(kv)) result.push_back(kv);
        return result;
    }
    std::optional<std::pair<K, V>> findFirst(typename ContainerSystem::Core::Predicate<std::pair<K, V>> pred) const override {
        for (const auto& kv : getAll()) if (pred(kv)) return kv;
        return std::nullopt;
    }
    void sort(typename ContainerSystem::Core::Comparator<std::pair<K, V>> comp) override {
        // multimap is ordered by key; no-op
    }

    /**
     * @brief Bulk-inserts entries from an input range while preserving duplicates.
     * @tparam R Range convertible to std::pair<K, V>.
     */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, std::pair<K, V>>
    void insert_range(R&& range) {
        for (auto&& entry : range) {
            std::pair<K, V> kv = static_cast<std::pair<K, V>>(entry);
            data.emplace(std::move(kv.first), std::move(kv.second));
        }
    }
};

} // namespace ContainerSystem::Associative

#endif // CONTAINERS_ASSOCIATIVE_SOULMULTIMAP_H