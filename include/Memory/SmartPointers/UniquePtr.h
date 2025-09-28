#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include "Core/IMemoryManager.h"
#include "Core/MemoryRegistry.h"
#include "Core/MemoryTag.h"

namespace Memory::Smart {

template<typename T>
class UniquePtr {
public:
    struct Deleter {
        Memory::Core::IMemoryManager* manager {nullptr};

        void operator()(T* ptr) const {
            if (!ptr || !manager) {
                return;
            }
            manager->unregisterAllocation(ptr);
            delete ptr;
        }
    };

    using pointer = T*;

    explicit UniquePtr(std::unique_ptr<T, Deleter> ptr) : ptr_(std::move(ptr)) {}

    template<typename... Args>
    static UniquePtr<T> createWithArgs(const std::string& tag, Args&&... args) {
        auto* manager = &Memory::Core::MemoryRegistry::Get();
        const auto tagValue = tag.empty()
            ? Memory::Core::MakeRuntimeTag(typeid(T).name())
            : Memory::Core::MakeRuntimeTag(tag);

        T* raw = new T(std::forward<Args>(args)...);
        manager->registerAllocation(raw, sizeof(T), tagValue);
        return UniquePtr<T>(std::unique_ptr<T, Deleter>(raw, Deleter{manager}));
    }

    static UniquePtr<T> create(const std::string& tag = "") {
        return createWithArgs(tag);
    }

    T* get() const { return ptr_.get(); }
    T* operator->() const { return ptr_.get(); }
    T& operator*() const { return *ptr_; }
    pointer release() { return ptr_.release(); }
    void reset(pointer p = nullptr) { ptr_.reset(p); }

private:
    std::unique_ptr<T, Deleter> ptr_;
};

} // namespace Memory::Smart
