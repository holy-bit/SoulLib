#ifndef CONTAINERS_ASSOCIATIVE_SOULMAP_H
#define CONTAINERS_ASSOCIATIVE_SOULMAP_H

#include <concepts>
#include <map>
#include <optional>
#include <ranges>
#include <vector>
#include "containers/Core/IContainer.h"

namespace ContainerSystem::Associative {

/**
 * @brief Ordered map adapter that wraps std::map behind the Soul container API.
 * @tparam K Key type.
 * @tparam V Mapped value type.
 */
template<typename K, typename V>
class SoulMap : public ContainerSystem::Core::IContainer<std::pair<K, V>> {
private:
    std::map<K, V> data;
public:
    SoulMap() = default;

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, std::pair<K, V>>
    explicit SoulMap(R&& range) {
        insert_range(std::forward<R>(range));
    }

    /// Inserts or overwrites the mapping for the given key.
    void insert(const std::pair<K, V>& kv) override {
        data[kv.first] = kv.second;
    }
    /// Removes the entry identified by the key from @p kv.
    void remove(const std::pair<K, V>& kv) override {
        data.erase(kv.first);
    }
    /// Returns all key-value pairs ordered by key.
    std::vector<std::pair<K, V>> getAll() const override {
        std::vector<std::pair<K, V>> result;
        for (auto& [k,v] : data) result.emplace_back(k, v);
        return result;
    }
    size_t size() const override { return data.size(); }
    std::vector<std::pair<K, V>> findAll(typename ContainerSystem::Core::Predicate<std::pair<K, V>> pred) const override {
        std::vector<std::pair<K, V>> result;
        for (auto& kv : getAll()) if (pred(kv)) result.push_back(kv);
        return result;
    }
    std::optional<std::pair<K, V>> findFirst(typename ContainerSystem::Core::Predicate<std::pair<K, V>> pred) const override {
        for (auto& kv : getAll()) if (pred(kv)) return kv;
        return std::nullopt;
    }
    void sort(typename ContainerSystem::Core::Comparator<std::pair<K, V>> comp) override {
        // map is ordered by key; no-op
    }

    /**
     * @brief Inserts or overwrites entries from an input range.
     * @tparam R Range convertible to std::pair<K, V>.
     */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, std::pair<K, V>>
    void insert_range(R&& range) {
        for (auto&& entry : range) {
            std::pair<K, V> kv = static_cast<std::pair<K, V>>(entry);
            insert(kv);
        }
    }
};

} // namespace ContainerSystem::Associative

#endif // CONTAINERS_ASSOCIATIVE_SOULMAP_H