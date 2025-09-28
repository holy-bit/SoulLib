#ifndef CONTAINERS_CORE_ICONTAINER_H
#define CONTAINERS_CORE_ICONTAINER_H

#include <vector>
#include <functional>
#include <optional>

namespace ContainerSystem::Core {

template<typename T>
using Predicate = std::function<bool(const T&)>;

template<typename T>
using Comparator = std::function<bool(const T&, const T&)>;

template<typename T>
class IContainer {
public:
    virtual ~IContainer() = default;
    virtual void insert(const T& value) = 0;
    virtual void remove(const T& value) = 0;
    virtual std::vector<T> getAll() const = 0;
    virtual size_t size() const = 0;
    virtual std::vector<T> findAll(Predicate<T> pred) const = 0;
    virtual std::optional<T> findFirst(Predicate<T> pred) const = 0;
    virtual void sort(Comparator<T> comp) = 0;
};

} // namespace ContainerSystem::Core

#endif // CONTAINERS_CORE_ICONTAINER_H