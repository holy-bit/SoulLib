#pragma once

#include "Memory/Core/MemoryRegistry.h"
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ContainerSystem::Core {

class ContainerMemoryOptimizer {
public:
    template<typename T, typename Allocator>
    static void optimize(std::vector<T, Allocator>& container) {
        std::vector<T, Allocator>(container).swap(container);
    }

    template<typename T, typename Allocator>
    static void optimize(std::deque<T, Allocator>& container) {
        std::deque<T, Allocator> temp;
        for (const auto& item : container) {
            temp.push_back(item);
        }
        container.swap(temp);
    }

    template<typename T, typename Allocator>
    static void optimize(std::list<T, Allocator>&) {
        // Lists manage allocations per node; no generic optimization.
    }

    template<typename T, typename Allocator>
    static void optimize(std::forward_list<T, Allocator>&) {
        // Forward lists are already minimal by design.
    }

    template<typename K, typename V, typename Hash, typename KeyEqual, typename Allocator>
    static void optimize(std::unordered_map<K, V, Hash, KeyEqual, Allocator>& container) {
        container.rehash(0);
    }

    template<typename T, typename Hash, typename KeyEqual, typename Allocator>
    static void optimize(std::unordered_set<T, Hash, KeyEqual, Allocator>& container) {
        container.rehash(0);
    }

    template<typename Container>
    static void optimizeAndReport(Container& container, const std::string& containerName) {
        auto& memManager = Memory::Core::MemoryRegistry::Get();
        size_t beforeSize = memManager.getTotalAllocated();

        optimize(container);

        size_t afterSize = memManager.getTotalAllocated();
        std::cout << "Memory optimization for " << containerName << ": "
                  << beforeSize - afterSize << " bytes saved" << std::endl;
    }
};

} // namespace ContainerSystem::Core
