#pragma once
#include <memory>

namespace Memory::Smart {

template<typename T>
class WeakPtr {
public:
    WeakPtr() = default;
    WeakPtr(const std::weak_ptr<T>& wptr) : wptr_(wptr) {}
    explicit WeakPtr(const std::shared_ptr<T>& sptr) : wptr_(sptr) {}

    std::shared_ptr<T> lock() const { return wptr_.lock(); }
    bool expired() const { return wptr_.expired(); }

private:
    std::weak_ptr<T> wptr_;
};

} // namespace Memory::Smart