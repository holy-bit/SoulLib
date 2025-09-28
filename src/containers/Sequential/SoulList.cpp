#include "containers/Sequential/SoulList.h"
#include <algorithm>
#include <optional>
#include <string>
#include <iterator> // Para std::back_inserter

namespace ContainerSystem::Sequential {

template<typename T>
void SoulList<T>::insert(const T& value) { data.push_back(value); }

template<typename T>
void SoulList<T>::remove(const T& value) {
    data.remove(value);
}

template<typename T>
std::vector<T> SoulList<T>::getAll() const { 
    std::vector<T> result;
    result.reserve(data.size());
    std::copy(data.begin(), data.end(), std::back_inserter(result));
    return result;
}

template<typename T>
size_t SoulList<T>::size() const { return data.size(); }

template<typename T>
std::vector<T> SoulList<T>::findAll(ContainerSystem::Core::Predicate<T> pred) const {
    std::vector<T> result;
    for (const auto& item : data) {
        if (pred(item)) {
            result.push_back(item);
        }
    }
    return result;
}

template<typename T>
std::optional<T> SoulList<T>::findFirst(ContainerSystem::Core::Predicate<T> pred) const {
    for (const auto& item : data) {
        if (pred(item)) {
            return item;
        }
    }
    return std::nullopt;
}

template<typename T>
void SoulList<T>::sort(ContainerSystem::Core::Comparator<T> comp) {
    data.sort(comp);
}

// Explicit instantiation if needed
template class SoulList<int>;
template class SoulList<std::string>;

} // namespace ContainerSystem::Sequential