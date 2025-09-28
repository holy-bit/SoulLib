#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/MemoryRegistry.h"
#include "containers/ContainerSystem.h"
#include "containers/Sequential/SoulDeque.h"
#include "containers/Sequential/SoulVector.h"
#include "containers/Unordered/SoulUnorderedMap.h"
#include "containers/Associative/SoulFlatMap.h"
#include "containers/Utility/ContainerMemoryUtils.h"
#include "debug/Debug.h"
#include <iostream>
#include <string>

// Example demonstrating memory-optimized container usage
int main() {
    // Install a local memory manager for this demo and enable debug tracing.
    Memory::Core::MemoryManager localManager;
    localManager.setDebugMode(true);
    Memory::Core::MemoryRegistry::Set(localManager);

    // Create some containers with our memory-optimized implementation
    ContainerSystem::Sequential::SoulVector<int> myVector;
    ContainerSystem::Sequential::SoulDeque<double> myDeque;
    ContainerSystem::Unordered::SoulUnorderedMap<std::string, int> myMap;
    ContainerSystem::Associative::SoulFlatMap<std::string, int> myFlatMap;

    DEBUG_LOG("=== Initial Container Memory Usage ===");
    ContainerSystem::getContainerMemoryProfiler().reportContainerMemoryUsage();

    // Fill containers with data
    myVector.reserve(1000);
    for (int i = 0; i < 1000; i++) {
        myVector.insert(i);
    }

    for (int i = 0; i < 500; i++) {
        myDeque.insert(i * 1.5);
    }

    for (int i = 0; i < 300; i++) {
        myMap.insert(std::make_pair("key" + std::to_string(i), i));
    }

    for (int i = 0; i < 32; ++i) {
        myFlatMap.insert({"asset_" + std::to_string(i), i});
    }

    DEBUG_LOG("=== Container Memory Usage After Filling ===");
    ContainerSystem::getContainerMemoryProfiler().reportContainerMemoryUsage();

    // Demonstrate individual container memory optimization
    DEBUG_LOG("=== Optimizing Individual Containers ===");
    ContainerSystem::Utility::optimizeMemory(myVector, "myVector");

    // Remove half the elements from vector and optimize again
    for (int i = 0; i < 500; i++) {
        myVector.remove(i);
    }

    ContainerSystem::Utility::optimizeMemory(myVector, "myVector after removal");

    // Demonstrate SBO recovery by shrinking after temporary growth
    for (int i = 0; i < 64; ++i) {
        myVector.insert(1000 + i);
    }
    ContainerSystem::Utility::optimizeMemory(myVector, "myVector after SBO stress");

    // Batch optimize multiple containers
    DEBUG_LOG("=== Batch Optimizing Multiple Containers ===");
    ContainerSystem::Utility::batchOptimize(myDeque, myMap, myFlatMap);

    DEBUG_LOG("=== Final Container Memory Usage ===");
    ContainerSystem::getContainerMemoryProfiler().reportContainerMemoryUsage();

    // Report any memory leaks
    localManager.reportLeaks();

    return 0;
}
