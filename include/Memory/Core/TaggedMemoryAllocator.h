#pragma once

#include "IMemoryManager.h"
#include "MemoryRegistry.h"
#include <cstddef>
#include <new>
#include <type_traits>

namespace Memory::Core {
    template<typename T, typename Tag>
    class TaggedMemoryAllocator {
    public:
        using value_type = T;
        using tag_type = Tag;

        TaggedMemoryAllocator() noexcept
            : manager_(&MemoryRegistry::Get()) {}

        template<typename U>
        TaggedMemoryAllocator(const TaggedMemoryAllocator<U, Tag>& other) noexcept
            : manager_(other.manager_) {}

        explicit TaggedMemoryAllocator(IMemoryManager& manager) noexcept
            : manager_(&manager) {}

        T* allocate(std::size_t n) {
            std::size_t bytes = n * sizeof(T);
            T* ptr = static_cast<T*>(::operator new(bytes));
            manager_->registerAllocation(ptr, bytes, Tag::value());
            return ptr;
        }

        void deallocate(T* ptr, std::size_t) noexcept {
            if (!ptr) {
                return;
            }
            manager_->unregisterAllocation(ptr);
            ::operator delete(ptr);
        }

        template<typename U>
        struct rebind { using other = TaggedMemoryAllocator<U, Tag>; };

        IMemoryManager* manager() const noexcept { return manager_; }

    private:
        template<typename, typename>
        friend class TaggedMemoryAllocator;
        IMemoryManager* manager_ {nullptr};
    };

    template<typename T, typename Tag, typename U, typename UTag>
    inline bool operator==(const TaggedMemoryAllocator<T, Tag>& lhs, const TaggedMemoryAllocator<U, UTag>& rhs) noexcept {
        return std::is_same_v<Tag, UTag> && lhs.manager() == rhs.manager();
    }

    template<typename T, typename Tag, typename U, typename UTag>
    inline bool operator!=(const TaggedMemoryAllocator<T, Tag>& a, const TaggedMemoryAllocator<U, UTag>& b) noexcept {
        return !(a == b);
    }
}
