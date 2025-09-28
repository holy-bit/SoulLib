#pragma once

#include <iosfwd>

#include "Memory/Core/MemoryStatistics.h"

namespace Memory::Profiler {

/**
 * @brief Utility helpers to export memory statistics for external tooling.
 */
class MemoryVisualizer {
public:
    /**
     * @brief Writes the statistics as JSON formatted output.
     * @param stats Aggregated memory metrics sampled from the registry.
     * @param output Target stream that receives the JSON document.
     */
    static void WriteJson(const Core::MemoryStatistics& stats, std::ostream& output);

    /**
     * @brief Emits a Graphviz DOT representation of the memory tags tree.
     * @param stats Aggregated memory metrics sampled from the registry.
     * @param output Target stream that receives the DOT graph.
     */
    static void WriteGraphviz(const Core::MemoryStatistics& stats, std::ostream& output);
};

} // namespace Memory::Profiler
