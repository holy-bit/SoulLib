#pragma once

#include "IMemoryManager.h"
#include "MemoryRegistry.h"
#include <cstddef>
#include <new>

namespace Memory::Core {
    namespace detail {
        constexpr MemoryTag kDefaultAllocatorTag = SOUL_MEMORY_TAG("MemoryAllocator");
    }

    template<typename T>
    class MemoryAllocator {
    public:
        using value_type = T;

        MemoryAllocator() noexcept
            : manager_(&MemoryRegistry::Get()) {}

        template<typename U>
        MemoryAllocator(const MemoryAllocator<U>& other) noexcept
            : manager_(other.manager_) {}

        explicit MemoryAllocator(IMemoryManager& manager) noexcept
            : manager_(&manager) {}

        T* allocate(std::size_t n) {
            std::size_t bytes = n * sizeof(T);
            T* ptr = static_cast<T*>(::operator new(bytes));
            manager_->registerAllocation(ptr, bytes, detail::kDefaultAllocatorTag);
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
        struct rebind { using other = MemoryAllocator<U>; };

        IMemoryManager* manager() const noexcept { return manager_; }

    private:
        template<typename> friend class MemoryAllocator;
        IMemoryManager* manager_ {nullptr};
    };

    template<typename T, typename U>
    inline bool operator==(const MemoryAllocator<T>& lhs, const MemoryAllocator<U>& rhs) noexcept {
        return lhs.manager() == rhs.manager();
    }

    template<typename T, typename U>
    inline bool operator!=(const MemoryAllocator<T>& a, const MemoryAllocator<U>& b) noexcept {
        return !(a == b);
    }
}
