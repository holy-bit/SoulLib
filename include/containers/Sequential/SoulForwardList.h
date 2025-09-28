#ifndef CONTAINERS_SEQUENTIAL_SOULFORWARDLIST_H
#define CONTAINERS_SEQUENTIAL_SOULFORWARDLIST_H

#include <forward_list>
#include <optional>
#include <string>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"
#include "containers/Core/ContainerTags.h"
#include "Memory/Core/TaggedMemoryAllocator.h"

namespace ContainerSystem::Sequential {

template<typename T>
class SoulForwardList : public ContainerSystem::Core::IContainer<T> {
private:
    std::forward_list<T, Memory::Core::TaggedMemoryAllocator<T, ContainerSystem::Core::ForwardListTag>> data;
public:
    void insert(const T& value) override;
    void remove(const T& value) override;
    std::vector<T> getAll() const override;
    size_t size() const override;
    std::vector<T> findAll(typename ContainerSystem::Core::Predicate<T> pred) const override;
    std::optional<T> findFirst(typename ContainerSystem::Core::Predicate<T> pred) const override;
    void sort(typename ContainerSystem::Core::Comparator<T> comp) override;
};

} // namespace ContainerSystem::Sequential

#endif // CONTAINERS_SEQUENTIAL_SOULFORWARDLIST_H