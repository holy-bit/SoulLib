#ifndef CONTAINERS_ALGORITHMS_SORTER_H
#define CONTAINERS_ALGORITHMS_SORTER_H

#include <vector>
#include <algorithm>
#include "containers/Core/IContainer.h"

namespace ContainerSystem::Algorithms {

class Sorter {
public:
    template<typename T>
    static void sortByComparator(std::vector<T>& vec, const Core::Comparator<T>& comp) {
        std::sort(vec.begin(), vec.end(), comp);
    }

    template<typename T>
    static void sortAscending(std::vector<T>& vec) {
        std::sort(vec.begin(), vec.end());
    }

    template<typename T>
    static void sortDescending(std::vector<T>& vec) {
        std::sort(vec.begin(), vec.end(), std::greater<T>());
    }

    template<typename T>
    static void stableSort(std::vector<T>& vec, const Core::Comparator<T>& comp) {
        std::stable_sort(vec.begin(), vec.end(), comp);
    }
};

} // namespace ContainerSystem::Algorithms

#endif // CONTAINERS_ALGORITHMS_SORTER_H