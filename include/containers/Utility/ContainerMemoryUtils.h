#pragma once

#include "Memory/Core/MemoryRegistry.h"
#include "containers/Core/ContainerMemoryOptimizer.h"
#include "containers/Sequential/SoulDeque.h"
#include "containers/Sequential/SoulForwardList.h"
#include "containers/Sequential/SoulList.h"
#include "containers/Sequential/SoulVector.h"
#include "containers/Unordered/SoulUnorderedMap.h"
#include "containers/Unordered/SoulUnorderedSet.h"
#include <iostream>
#include <string>
#include <type_traits>

namespace ContainerSystem::Utility {

// Optimize memory for any container type
template<typename Container>
void optimizeMemory(Container& container, const std::string& name = "Container") {
    Core::ContainerMemoryOptimizer::optimizeAndReport(container, name);
}

// Specialized optimization for SoulVector to use its built-in shrink_to_fit
template<typename T>
void optimizeMemory(Sequential::SoulVector<T>& vector, const std::string& name = "SoulVector") {
    std::cout << "Optimizing memory for " << name << "..." << std::endl;
    auto& memManager = Memory::Core::MemoryRegistry::Get();
    size_t beforeSize = memManager.getTotalAllocated();

    vector.shrink_to_fit();

    size_t afterSize = memManager.getTotalAllocated();
    std::cout << "Memory optimization for " << name << ": "
              << beforeSize - afterSize << " bytes saved" << std::endl;
}

// General container memory usage diagnostics
template<typename Container>
void reportMemoryUsage(const Container& container, const std::string& name = "Container") {
    std::cout << "Memory usage for " << name << ": "
              << sizeof(Container) + sizeof(typename Container::value_type) * container.size()
              << " bytes (estimated)" << std::endl;
}

// Helper function to batch optimize multiple containers
template<typename... Containers>
void batchOptimize(Containers&... containers) {
    (optimizeMemory(containers), ...);
}

} // namespace ContainerSystem::Utility
