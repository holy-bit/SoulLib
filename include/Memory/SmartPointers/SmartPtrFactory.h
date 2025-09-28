#pragma once

#include "SharedPtr.h"
#include "UniquePtr.h"
#include "WeakPtr.h"
#include <typeinfo>
#include <utility>

namespace Memory::Smart {

class SmartPtrFactory {
public:
    template<typename T, typename... Args>
    static UniquePtr<T> makeUnique(Args&&... args) {
        return UniquePtr<T>::createWithArgs(typeid(T).name(), std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    static SharedPtr<T> makeShared(Args&&... args) {
        return SharedPtr<T>::createWithArgs(typeid(T).name(), std::forward<Args>(args)...);
    }

    template<typename T>
    static WeakPtr<T> makeWeak(const SharedPtr<T>& shared) {
        return shared.getWeak();
    }
};

} // namespace Memory::Smart
