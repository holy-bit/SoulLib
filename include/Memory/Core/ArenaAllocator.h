#pragma once

#include "IMemoryManager.h"
#include "MemoryRegistry.h"
#include "MemoryTag.h"
#include <cstddef>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Memory::Core {

class ArenaAllocator {
public:
    explicit ArenaAllocator(std::size_t capacity,
                            MemoryTag tag = SOUL_MEMORY_TAG("ArenaAllocator"))
        : ArenaAllocator(MemoryRegistry::Get(), capacity, tag) {}

    ArenaAllocator(IMemoryManager& manager,
                   std::size_t capacity,
                   MemoryTag tag = SOUL_MEMORY_TAG("ArenaAllocator"))
        : manager_(&manager), tag_(tag), capacity_(capacity) {
        if (capacity_ == 0) {
            throw std::invalid_argument("ArenaAllocator capacity must be greater than zero");
        }
        allocateArena();
    }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;
    ArenaAllocator(ArenaAllocator&&) = delete;
    ArenaAllocator& operator=(ArenaAllocator&&) = delete;

    ~ArenaAllocator() {
        if (buffer_) {
            manager_->unregisterAllocation(buffer_);
            ::operator delete(buffer_, std::align_val_t(kAlignment));
        }
    }

    void* allocate(std::size_t size, std::size_t alignment = alignof(std::max_align_t)) {
        std::size_t current = align(offset_, alignment);
        if (current + size > capacity_) {
            throw std::bad_alloc();
        }
        char* address = static_cast<char*>(buffer_) + current;
        offset_ = current + size;
        return address;
    }

    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        return new (mem) T(std::forward<Args>(args)...);
    }

    void reset() noexcept { offset_ = 0; }

    std::size_t capacity() const noexcept { return capacity_; }
    std::size_t used() const noexcept { return offset_; }
    std::size_t remaining() const noexcept { return capacity_ - offset_; }

private:
    static constexpr std::size_t kAlignment = alignof(std::max_align_t);

    static std::size_t align(std::size_t value, std::size_t alignment) {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    void allocateArena() {
        buffer_ = ::operator new(capacity_, std::align_val_t(kAlignment));
        manager_->registerAllocation(buffer_, capacity_, tag_);
        offset_ = 0;
    }

    IMemoryManager* manager_ {nullptr};
    MemoryTag tag_;
    void* buffer_ {nullptr};
    std::size_t capacity_ {0};
    std::size_t offset_ {0};
};

} // namespace Memory::Core
