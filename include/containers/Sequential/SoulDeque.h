#ifndef CONTAINERS_SEQUENTIAL_SOULDEQUE_H
#define CONTAINERS_SEQUENTIAL_SOULDEQUE_H

#include <deque>
#include "containers/Core/IContainer.h"
#include "containers/Algorithms/Sorter.h"
#include "containers/Algorithms/Searcher.h"
#include "containers/Core/ContainerTags.h"
#include "Memory/Core/TaggedMemoryAllocator.h"

namespace ContainerSystem::Sequential {

template<typename T>
class SoulDeque : public ContainerSystem::Core::IContainer<T> {
private:
    std::deque<T, Memory::Core::TaggedMemoryAllocator<T, ContainerSystem::Core::DequeTag>> data;
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

#endif // CONTAINERS_SEQUENTIAL_SOULDEQUE_H