#include "containers/Sequential/SoulDeque.h"
#include <algorithm>
#include <iterator>

namespace ContainerSystem::Sequential {

template<typename T>
void SoulDeque<T>::insert(const T& value) { data.push_back(value); }

template<typename T>
void SoulDeque<T>::remove(const T& value) {
    data.erase(std::remove(data.begin(), data.end(), value), data.end());
}

template<typename T>
std::vector<T> SoulDeque<T>::getAll() const { 
    std::vector<T> result;
    result.reserve(data.size());
    result.insert(result.end(), data.begin(), data.end());
    return result;
}

template<typename T>
size_t SoulDeque<T>::size() const { return data.size(); }

template<typename T>
std::vector<T> SoulDeque<T>::findAll(ContainerSystem::Core::Predicate<T> pred) const {
    std::vector<T> result;
    for (const auto& item : data) {
        if (pred(item)) {
            result.push_back(item);
        }
    }
    return result;
}

template<typename T>
std::optional<T> SoulDeque<T>::findFirst(ContainerSystem::Core::Predicate<T> pred) const {
    for (const auto& item : data) {
        if (pred(item)) {
            return item;
        }
    }
    return std::nullopt;
}

template<typename T>
void SoulDeque<T>::sort(ContainerSystem::Core::Comparator<T> comp) {
    auto vec = getAll();
    ContainerSystem::Algorithms::Sorter::sortByComparator(vec, comp);
    data.assign(vec.begin(), vec.end());
}

// Explicit instantiation if needed
template class SoulDeque<int>;
template class SoulDeque<std::string>;

} // namespace ContainerSystem::Sequential