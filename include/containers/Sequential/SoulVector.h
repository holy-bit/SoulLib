#pragma once

#include "Memory/Core/TaggedMemoryAllocator.h"
#include "containers/Core/ContainerTags.h"
#include "containers/Core/IContainer.h"
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace ContainerSystem::Sequential {

template<typename T,
         std::size_t SmallCapacity = 16,
         typename Allocator = Memory::Core::TaggedMemoryAllocator<T, ContainerSystem::Core::VectorTag>>
class SoulVector : public ContainerSystem::Core::IContainer<T> {
public:
    static_assert(SmallCapacity > 0, "SoulVector requires SmallCapacity > 0");

    using value_type = T;
    using allocator_type = Allocator;

    SoulVector()
        : SoulVector(allocator_type{}) {}

    explicit SoulVector(const allocator_type& allocator)
        : allocator_(allocator),
          data_(reinterpret_cast<T*>(smallStorage_)),
          size_(0),
          capacity_(SmallCapacity) {}

    SoulVector(std::initializer_list<T> init)
        : SoulVector() {
        reserve(init.size());
        for (const auto& value : init) {
            emplace_back_internal(value);
        }
    }

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    explicit SoulVector(R&& range)
        : SoulVector() {
        append_range(std::forward<R>(range));
    }

    SoulVector(const SoulVector& other)
        : SoulVector(other.allocator_) {
        reserve(other.size_);
        for (const auto& value : other) {
            emplace_back_internal(value);
        }
    }

    SoulVector(SoulVector&& other) noexcept
        : allocator_(other.allocator_) {
        adopt_from(std::move(other));
    }

    SoulVector& operator=(const SoulVector& other) {
        if (this != &other) {
            SoulVector tmp(other);
            swap(tmp);
        }
        return *this;
    }

    SoulVector& operator=(SoulVector&& other) noexcept {
        if (this != &other) {
            release_storage();
            allocator_ = other.allocator_;
            adopt_from(std::move(other));
        }
        return *this;
    }

    ~SoulVector() { release_storage(); }

    void insert(const T& value) override { emplace_back_internal(value); }

    void remove(const T& value) override {
        auto newEnd = std::remove(begin(), end(), value);
        destroy_range(static_cast<std::size_t>(newEnd - begin()), size_);
        size_ = static_cast<std::size_t>(newEnd - begin());
    }

    T* erase(const T* pos) {
        T* p = const_cast<T*>(pos);
        if (p < begin() || p >= end()) return end();
        
        for (T* it = p; it < end() - 1; ++it) {
            *it = std::move(*(it + 1));
        }
        (end() - 1)->~T();
        --size_;
        return p;
    }

    std::vector<T> getAll() const override {
        std::vector<T> result;
        result.reserve(size_);
        for (const auto& value : *this) {
            result.push_back(value);
        }
        return result;
    }

    size_t size() const override { return size_; }

    std::vector<T> findAll(typename ContainerSystem::Core::Predicate<T> pred) const override {
        std::vector<T> matches;
        for (const auto& value : *this) {
            if (pred(value)) {
                matches.push_back(value);
            }
        }
        return matches;
    }

    std::optional<T> findFirst(typename ContainerSystem::Core::Predicate<T> pred) const override {
        for (const auto& value : *this) {
            if (pred(value)) {
                return value;
            }
        }
        return std::nullopt;
    }

    void sort(typename ContainerSystem::Core::Comparator<T> comp) override {
        std::sort(begin(), end(), comp);
    }

    void reserve(std::size_t newCapacity) {
        if (newCapacity <= capacity_) {
            return;
        }
        reallocate(newCapacity);
    }

    void shrink_to_fit() {
        if (size_ <= SmallCapacity && !using_small_storage()) {
            move_to_small();
            return;
        }
        if (size_ < capacity_ && !using_small_storage()) {
            reallocate(size_);
        }
    }

    void clear() {
        destroy_range(0, size_);
        size_ = 0;
    }

    template<std::ranges::input_range R>
    requires std::convertible_to<std::ranges::range_reference_t<R>, T>
    void append_range(R&& range) {
        if constexpr (std::ranges::sized_range<R>) {
            const auto additional = static_cast<std::size_t>(std::ranges::size(range));
            if (additional > 0) {
                reserve(size_ + additional);
            }
        }
        for (auto&& element : range) {
            emplace_back_internal(std::forward<decltype(element)>(element));
        }
    }

    T* data() noexcept { return data_; }
    const T* data() const noexcept { return data_; }

    T* begin() noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }
    const T* begin() const noexcept { return data_; }
    const T* end() const noexcept { return data_ + size_; }

    allocator_type get_allocator() const noexcept { return allocator_; }

    void swap(SoulVector& other) noexcept {
        if (this == &other) {
            return;
        }
        SoulVector tmp(std::move(other));
        other = std::move(*this);
        *this = std::move(tmp);
    }

private:
    allocator_type allocator_ {};
    T* data_ {reinterpret_cast<T*>(smallStorage_)};
    std::size_t size_ {0};
    std::size_t capacity_ {SmallCapacity};
    alignas(T) unsigned char smallStorage_[sizeof(T) * SmallCapacity];

    bool using_small_storage() const noexcept { return data_ == reinterpret_cast<const T*>(smallStorage_); }

    void release_storage() {
        destroy_range(0, size_);
        if (!using_small_storage()) {
            allocator_.deallocate(data_, capacity_);
        }
        data_ = reinterpret_cast<T*>(smallStorage_);
        capacity_ = SmallCapacity;
        size_ = 0;
    }

    void adopt_from(SoulVector&& other) noexcept {
        if (other.using_small_storage()) {
            data_ = reinterpret_cast<T*>(smallStorage_);
            capacity_ = SmallCapacity;
            size_ = 0;
            for (std::size_t i = 0; i < other.size_; ++i) {
                emplace_back_internal(std::move(other.data_[i]));
                other.data_[i].~T();
            }
            other.size_ = 0;
        } else {
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = reinterpret_cast<T*>(other.smallStorage_);
            other.size_ = 0;
            other.capacity_ = SmallCapacity;
        }
    }

    void destroy_range(std::size_t beginIndex, std::size_t endIndex) noexcept {
        for (std::size_t i = beginIndex; i < endIndex; ++i) {
            data_[i].~T();
        }
    }

    template<typename U>
    void emplace_back_internal(U&& value) {
        if (size_ == capacity_) {
            const std::size_t desired = capacity_ == 0 ? SmallCapacity : capacity_ * 2;
            reallocate(desired);
        }
        new (data_ + size_) T(std::forward<U>(value));
        ++size_;
    }

    void reallocate(std::size_t requestedCapacity) {
        std::size_t targetCapacity = requestedCapacity;
        if (targetCapacity <= SmallCapacity) {
            targetCapacity = SmallCapacity;
        }

        const bool moveToSmall = targetCapacity <= SmallCapacity;
        if (moveToSmall && using_small_storage()) {
            return;
        }

        if (moveToSmall) {
            move_to_small();
            return;
        }

        T* newData = allocator_.allocate(targetCapacity);
        std::size_t i = 0;
        try {
            for (; i < size_; ++i) {
                new (newData + i) T(std::move_if_noexcept(data_[i]));
            }
        } catch (...) {
            for (std::size_t j = 0; j < i; ++j) {
                newData[j].~T();
            }
            allocator_.deallocate(newData, targetCapacity);
            throw;
        }

        destroy_range(0, size_);
        if (!using_small_storage()) {
            allocator_.deallocate(data_, capacity_);
        }

        data_ = newData;
        capacity_ = targetCapacity;
    }

    void move_to_small() {
        if (using_small_storage()) {
            return;
        }

        T* destination = reinterpret_cast<T*>(smallStorage_);
        for (std::size_t i = 0; i < size_; ++i) {
            new (destination + i) T(std::move_if_noexcept(data_[i]));
            data_[i].~T();
        }
        allocator_.deallocate(data_, capacity_);
        data_ = destination;
        capacity_ = SmallCapacity;
    }
};

} // namespace ContainerSystem::Sequential

