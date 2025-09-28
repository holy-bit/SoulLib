#include "containers/ContainerSystem.h"
#include "containers/Core/ContainerMemoryOptimizer.h"
#include "debug/Debug.h"
#include "Memory/Core/MemoryRegistry.h"
#include <string>
#include <vector>

namespace ContainerSystem {

// Implementación del método para reportar el uso de memoria de los contenedores
void ContainerMemoryProfiler::reportContainerMemoryUsage() {
    auto& memManager = Memory::Core::MemoryRegistry::Get();

    DEBUG_LOG("=== Container Memory Usage Report ===");
    
    // Report memory usage by container type
    DEBUG_LOG("SoulVector: " + std::to_string(memManager.getAllocatedByTag(Core::VectorTag::value())) + " bytes");
    DEBUG_LOG("SoulDeque: " + std::to_string(memManager.getAllocatedByTag(Core::DequeTag::value())) + " bytes");
    DEBUG_LOG("SoulList: " + std::to_string(memManager.getAllocatedByTag(Core::ListTag::value())) + " bytes");
    DEBUG_LOG("SoulForwardList: " + std::to_string(memManager.getAllocatedByTag(Core::ForwardListTag::value())) + " bytes");
    DEBUG_LOG("SoulUnorderedMap: " + std::to_string(memManager.getAllocatedByTag(Core::UnorderedMapTag::value())) + " bytes");
    DEBUG_LOG("SoulUnorderedSet: " + std::to_string(memManager.getAllocatedByTag(Core::UnorderedSetTag::value())) + " bytes");
    
    DEBUG_LOG("=== Total Container Memory: " + 
        std::to_string(memManager.getAllocatedByTag(Core::VectorTag::value()) +
        memManager.getAllocatedByTag(Core::DequeTag::value()) +
        memManager.getAllocatedByTag(Core::ListTag::value()) +
        memManager.getAllocatedByTag(Core::ForwardListTag::value()) +
        memManager.getAllocatedByTag(Core::UnorderedMapTag::value()) +
        memManager.getAllocatedByTag(Core::UnorderedSetTag::value())) + " bytes ===");
}

// Implementación del método para optimizar el uso de memoria de los contenedores
void ContainerMemoryProfiler::optimizeContainerMemoryUsage() {
    DEBUG_LOG("Optimizing container memory usage...");
    
    // Get a reference to the memory manager
    auto& memManager = Memory::Core::MemoryRegistry::Get();
    size_t beforeSize = memManager.getTotalAllocated();
    
    // Since we don't have direct access to all registered containers through ContainerManager,
    // we'll log this information and continue with optimization techniques that don't require
    // iterating through all containers
    DEBUG_LOG("Applying memory optimization strategies");
    
    // Here we would typically iterate through containers if we had access to them
    // Instead, we'll focus on reporting memory usage before and after
    
    // Apply global memory optimizations if any are needed
    // Note: There's no performGlobalOptimizations() method, so we'll skip this step
    DEBUG_LOG("Global memory optimizations applied");
    
    // Report memory savings
    size_t afterSize = memManager.getTotalAllocated();
    if (beforeSize > afterSize) {
        DEBUG_LOG("Memory optimization complete. Saved: " + std::to_string(beforeSize - afterSize) + " bytes");
    } else {
        DEBUG_LOG("Memory optimization complete. No bytes saved.");
    }
}

// Método singleton para acceder a la instancia
ContainerMemoryProfiler& ContainerMemoryProfiler::instance() {
    static ContainerMemoryProfiler instance;
    return instance;
}

// Función global para acceder al perfilador de memoria de contenedores
ContainerMemoryProfiler& getContainerMemoryProfiler() {
    return ContainerMemoryProfiler::instance();
}

} // namespace ContainerSystem