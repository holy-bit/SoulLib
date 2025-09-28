#pragma once
#include "containers/Core/IContainer.h"
#include "containers/Core/ContainerManager.h"
#include "containers/Core/ContainerTags.h"
#include "Memory/Core/MemoryManager.h"
#include "Memory/Core/TaggedMemoryAllocator.h"
#include "debug/Debug.h"

namespace ContainerSystem {

class ContainerMemoryProfiler {
public:
    // Método singleton para acceder a la instancia
    static ContainerMemoryProfiler& instance();

    // Reporta el uso de memoria de los diferentes tipos de contenedores
    void reportContainerMemoryUsage();

    // Optimiza el uso de memoria de todos los contenedores registrados
    void optimizeContainerMemoryUsage();

private:
    // Constructor y destructor privados para singleton
    ContainerMemoryProfiler() = default;
    ~ContainerMemoryProfiler() = default;
    
    // Eliminar constructor de copia y operador de asignación
    ContainerMemoryProfiler(const ContainerMemoryProfiler&) = delete;
    ContainerMemoryProfiler& operator=(const ContainerMemoryProfiler&) = delete;
};

// Función global para acceder al perfilador de memoria de contenedores
ContainerMemoryProfiler& getContainerMemoryProfiler();

} // namespace ContainerSystem