#pragma once

#include "Memory/Core/TaggedMemoryAllocator.h"
#include "containers/Core/ContainerTags.h"
#include "containers/Core/IContainer.h"
#include <algorithm>
#include <concepts>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace ContainerSystem::Associative {

/**
 * @brief Cache-friendly associative map backed by a flat vector storage.
 *
 * @tparam K Key type stored in sorted order.
 * @tparam V Value type associated with each key.
 * @tparam Compare Strict weak ordering applied to keys.
 */
template<typename K,
         typename V,
         typename Compare = std::less<K>>
class SoulFlatMap : public ContainerSystem::Core::IContainer<std::pair<K, V>> {
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<K, V>;
    using predicate_type = ContainerSystem::Core::Predicate<value_type>;
    using comparator_type = ContainerSystem::Core::Comparator<value_type>;
    using storage_allocator = Memory::Core::TaggedMemoryAllocator<value_type, ContainerSystem::Core::FlatMapTag>;
    using storage_type = std::vector<value_type, storage_allocator>;

    SoulFlatMap() = default;

    explicit SoulFlatMap(Compare compare)
        : compare_(compare) {}

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    explicit SoulFlatMap(R&& range)
        : SoulFlatMap() {
        insert_range(std::forward<R>(range));
    }

    /// Inserts or overwrites a key-value pair while preserving key ordering.
    void insert(const value_type& kv) override {
        auto it = lower_bound(kv.first);
        if (it != data_.end() && keys_equal(it->first, kv.first)) {
            it->second = kv.second;
        } else {
            data_.insert(it, kv);
        }
    }

    /// Erases the entry whose key matches @p kv.
    void remove(const value_type& kv) override {
        auto it = lower_bound(kv.first);
        if (it != data_.end() && keys_equal(it->first, kv.first)) {
            data_.erase(it);
        }
    }

    /// Returns a sorted snapshot of the flat storage.
    std::vector<value_type> getAll() const override {
        return {data_.begin(), data_.end()};
    }

    size_t size() const override { return data_.size(); }

    std::vector<value_type> findAll(predicate_type pred) const override {
        std::vector<value_type> matches;
        for (const auto& kv : data_) {
            if (pred(kv)) {
                matches.push_back(kv);
            }
        }
        return matches;
    }

    std::optional<value_type> findFirst(predicate_type pred) const override {
        for (const auto& kv : data_) {
            if (pred(kv)) {
                return kv;
            }
        }
        return std::nullopt;
    }

    void sort(comparator_type comp) override {
        std::stable_sort(data_.begin(), data_.end(), comp);
    }

    void reserve(std::size_t capacity) { data_.reserve(capacity); }

    void clear() noexcept { data_.clear(); }

    /**
     * @brief Inserts entries from an input range, reserving capacity when possible.
     * @tparam R Range convertible to value_type.
     */
    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
    void insert_range(R&& range) {
        if constexpr (std::ranges::sized_range<R>) {
            const auto additional = static_cast<std::size_t>(std::ranges::size(range));
            if (additional > 0) {
                reserve(data_.size() + additional);
            }
        }
        for (auto&& element : range) {
            value_type kv = static_cast<value_type>(element);
            insert(kv);
        }
    }

    /// Checks if the requested key exists in the map.
    bool contains(const key_type& key) const { return find_iterator(key) != data_.end(); }

    /// Looks up a key returning an optional value.
    std::optional<mapped_type> lookup(const key_type& key) const {
        auto it = find_iterator(key);
        if (it == data_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    /// Provides write access, inserting a default value when the key is new.
    mapped_type& operator[](const key_type& key) {
        auto it = lower_bound(key);
        if (it == data_.end() || !keys_equal(it->first, key)) {
            it = data_.insert(it, value_type(key, mapped_type{}));
        }
        return it->second;
    }

    /// Finds the value for @p key returning nullptr when absent.
    const mapped_type* find(const key_type& key) const {
        auto it = find_iterator(key);
        return it != data_.end() ? &it->second : nullptr;
    }

    /// Non-const overload of find().
    mapped_type* find(const key_type& key) {
        auto it = find_iterator(key);
        return it != data_.end() ? &it->second : nullptr;
    }

private:
    Compare compare_ {};
    storage_type data_ {};

    typename storage_type::iterator lower_bound(const key_type& key) {
        return std::lower_bound(data_.begin(), data_.end(), key,
                                [this](const value_type& entry, const key_type& value) {
                                    return compare_(entry.first, value);
                                });
    }

    typename storage_type::const_iterator lower_bound(const key_type& key) const {
        return std::lower_bound(data_.begin(), data_.end(), key,
                                [this](const value_type& entry, const key_type& value) {
                                    return compare_(entry.first, value);
                                });
    }

    typename storage_type::iterator find_iterator(const key_type& key) {
        auto it = lower_bound(key);
        if (it != data_.end() && keys_equal(it->first, key)) {
            return it;
        }
        return data_.end();
    }

    typename storage_type::const_iterator find_iterator(const key_type& key) const {
        auto it = lower_bound(key);
        if (it != data_.end() && keys_equal(it->first, key)) {
            return it;
        }
        return data_.end();
    }

    bool keys_equal(const key_type& lhs, const key_type& rhs) const {
        return !compare_(lhs, rhs) && !compare_(rhs, lhs);
    }
};

} // namespace ContainerSystem::Associative
