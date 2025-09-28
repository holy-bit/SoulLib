#pragma once

#include "IMemoryManager.h"
#include "MemoryRegistry.h"
#include "MemoryTag.h"
#include <algorithm>
#include <cstddef>
#include <new>
#include <stdexcept>

namespace Memory::Core {

template<typename T, std::size_t BlockCount, std::size_t Alignment = alignof(T)>
class PoolAllocator {
public:
    static_assert(BlockCount > 0, "PoolAllocator requires at least one block");
    static_assert(Alignment >= alignof(T), "Alignment must satisfy the type alignment");

    explicit PoolAllocator(MemoryTag tag = SOUL_MEMORY_TAG("PoolAllocator"))
        : PoolAllocator(MemoryRegistry::Get(), tag) {}

    PoolAllocator(IMemoryManager& manager, MemoryTag tag = SOUL_MEMORY_TAG("PoolAllocator"))
        : manager_(&manager), tag_(tag) {
        allocatePool();
    }

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;

    ~PoolAllocator() {
        if (memoryBlock_) {
            manager_->unregisterAllocation(memoryBlock_);
            ::operator delete[](memoryBlock_, std::align_val_t(Alignment));
        }
    }

    T* allocate() {
        if (!freeList_) {
            return nullptr;
        }
        FreeNode* node = freeList_;
        freeList_ = node->next;
        --available_;
        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* ptr) {
        if (!ptr) {
            return;
        }
        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freeList_;
        freeList_ = node;
        ++available_;
    }

    void reset() {
        buildFreeList(static_cast<char*>(memoryBlock_), blockStride());
    }

    std::size_t capacity() const noexcept { return BlockCount; }
    std::size_t available() const noexcept { return available_; }

private:
    struct FreeNode {
        FreeNode* next {nullptr};
    };

    void allocatePool() {
        const std::size_t stride = blockStride();
        const std::size_t totalSize = stride * BlockCount;
        memoryBlock_ = ::operator new[](totalSize, std::align_val_t(Alignment));
        manager_->registerAllocation(memoryBlock_, totalSize, tag_);
        buildFreeList(static_cast<char*>(memoryBlock_), stride);
    }

    void buildFreeList(char* start, std::size_t stride) {
        freeList_ = nullptr;
        available_ = BlockCount;
        for (std::size_t i = 0; i < BlockCount; ++i) {
            auto* node = reinterpret_cast<FreeNode*>(start + i * stride);
            node->next = freeList_;
            freeList_ = node;
        }
    }

    static constexpr std::size_t blockStride() noexcept {
        const std::size_t required = std::max(sizeof(T), sizeof(FreeNode));
        return ((required + Alignment - 1) / Alignment) * Alignment;
    }

    IMemoryManager* manager_ {nullptr};
    MemoryTag tag_;
    void* memoryBlock_ {nullptr};
    FreeNode* freeList_ {nullptr};
    std::size_t available_ {0};
};

} // namespace Memory::Core
