#include <gtest/gtest.h>

#include <sstream>

#include "Memory/Core/MemoryStatistics.h"
#include "Memory/Core/MemoryTag.h"
#include "Memory/Profiler/MemoryVisualizer.h"

namespace {

Memory::Core::MemoryStatistics make_sample_stats() {
    Memory::Core::MemoryStatistics stats;
    stats.totalBytes = 1024;
    stats.allocationCount = 4;

    Memory::Core::MemoryTagStats tagA;
    tagA.tag = Memory::Core::MakeRuntimeTag("Assets");
    tagA.bytes = 512;
    tagA.allocations = 2;

    Memory::Core::MemoryTagStats tagB;
    tagB.tag = Memory::Core::MakeRuntimeTag("Gameplay");
    tagB.bytes = 256;
    tagB.allocations = 1;

    stats.tags.push_back(tagA);
    stats.tags.push_back(tagB);
    return stats;
}

} // namespace

TEST(MemoryVisualizer, WriteJsonProducesValidStructure) {
    const auto stats = make_sample_stats();
    std::ostringstream buffer;

    Memory::Profiler::MemoryVisualizer::WriteJson(stats, buffer);

    const auto output = buffer.str();
    EXPECT_NE(output.find("\"totalBytes\": 1024"), std::string::npos);
    EXPECT_NE(output.find("\"allocationCount\": 4"), std::string::npos);
    EXPECT_NE(output.find("\"bytes\": 512"), std::string::npos);
    EXPECT_NE(output.find("\"allocations\": 1"), std::string::npos);
}

TEST(MemoryVisualizer, WriteGraphvizProducesEdgesPerTag) {
    const auto stats = make_sample_stats();
    std::ostringstream buffer;

    Memory::Profiler::MemoryVisualizer::WriteGraphviz(stats, buffer);

    const auto output = buffer.str();
    EXPECT_NE(output.find("digraph MemoryTags"), std::string::npos);
    EXPECT_NE(output.find("total -> tag0"), std::string::npos);
    EXPECT_NE(output.find("total -> tag1"), std::string::npos);
    EXPECT_NE(output.find("bytes: 512"), std::string::npos);
    EXPECT_NE(output.find("allocations: 4"), std::string::npos);
}
