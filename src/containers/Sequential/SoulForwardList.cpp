#include "containers/Sequential/SoulForwardList.h"
#include <algorithm>
#include <iterator> // Para std::back_inserter
#include <string>

namespace ContainerSystem::Sequential {

template<typename T>
void SoulForwardList<T>::insert(const T& value) { data.push_front(value); }

template<typename T>
void SoulForwardList<T>::remove(const T& value) { data.remove(value); }

template<typename T>
std::vector<T> SoulForwardList<T>::getAll() const { 
    std::vector<T> result;
    result.reserve(std::distance(data.begin(), data.end()));
    std::copy(data.begin(), data.end(), std::back_inserter(result));
    return result;
}

template<typename T>
size_t SoulForwardList<T>::size() const { return std::distance(data.begin(), data.end()); }

template<typename T>
std::vector<T> SoulForwardList<T>::findAll(ContainerSystem::Core::Predicate<T> pred) const {
    std::vector<T> result;
    for (const auto& item : data) {
        if (pred(item)) {
            result.push_back(item);
        }
    }
    return result;
}

template<typename T>
std::optional<T> SoulForwardList<T>::findFirst(ContainerSystem::Core::Predicate<T> pred) const {
    for (const auto& item : data) {
        if (pred(item)) {
            return item;
        }
    }
    return std::nullopt;
}

template<typename T>
void SoulForwardList<T>::sort(ContainerSystem::Core::Comparator<T> comp) {
    data.sort(comp);
}

// Explicit instantiation
template class SoulForwardList<int>;
template class SoulForwardList<std::string>;

} // namespace ContainerSystem::Sequential