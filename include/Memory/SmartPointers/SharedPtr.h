#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include "Core/IMemoryManager.h"
#include "Core/MemoryRegistry.h"
#include "Core/MemoryTag.h"
#include "WeakPtr.h"

namespace Memory::Smart {

template<typename T>
class SharedPtr {
public:
    using pointer = std::shared_ptr<T>;

    explicit SharedPtr(pointer ptr) : ptr_(std::move(ptr)) {}

    template<typename... Args>
    static SharedPtr<T> createWithArgs(const std::string& tag, Args&&... args) {
        auto* manager = &Memory::Core::MemoryRegistry::Get();
        const auto tagValue = tag.empty()
            ? Memory::Core::MakeRuntimeTag(typeid(T).name())
            : Memory::Core::MakeRuntimeTag(tag);

        T* raw = new T(std::forward<Args>(args)...);
        manager->registerAllocation(raw, sizeof(T), tagValue);
        auto deleter = [manager](T* p) {
            if (p) {
                manager->unregisterAllocation(p);
                delete p;
            }
        };
        return SharedPtr<T>(pointer(raw, deleter));
    }

    static SharedPtr<T> create(const std::string& tag = "") {
        return createWithArgs(tag);
    }

    size_t use_count() const { return ptr_.use_count(); }
    WeakPtr<T> getWeak() const { return WeakPtr<T>(ptr_); }
    T* get() const { return ptr_.get(); }
    T* operator->() const { return ptr_.get(); }
    T& operator*() const { return *ptr_; }

private:
    pointer ptr_;
};

} // namespace Memory::Smart
