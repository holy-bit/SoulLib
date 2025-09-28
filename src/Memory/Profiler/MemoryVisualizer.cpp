#include "Memory/Profiler/MemoryVisualizer.h"

#include <ostream>
#include <string>
#include <string_view>

namespace Memory::Profiler {

namespace {

static void WriteTagJson(const Core::MemoryTagStats& tagStats, std::ostream& output, bool isLast) {
    output << "    {\n"
           << "      \"hash\": " << tagStats.tag.hash << ",\n"
#ifdef SOUL_DEBUG
           << "      \"label\": \"" << std::string(tagStats.tag.label) << "\",\n"
#else
           << "      \"label\": \"\",\n"
#endif
           << "      \"bytes\": " << tagStats.bytes << ",\n"
           << "      \"allocations\": " << tagStats.allocations << "\n"
           << "    }" << (isLast ? "\n" : ",\n");
}

static std::string EscapeGraphvizLabel(std::string_view text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (const char ch : text) {
        switch (ch) {
        case '\\':
        case '"':
            escaped.push_back('\\');
            escaped.push_back(ch);
            break;
        case '\n':
        case '\r':
            escaped.append("\\n");
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

static void WriteTagGraphviz(const Core::MemoryTagStats& tagStats, std::ostream& output, std::size_t index) {
    output << "  tag" << index << " [label=\"hash: " << tagStats.tag.hash
           << "\\nbytes: " << tagStats.bytes
           << "\\nallocations: " << tagStats.allocations;
#ifdef SOUL_DEBUG
    if (!tagStats.tag.label.empty()) {
        output << "\\nlabel: " << EscapeGraphvizLabel(tagStats.tag.label);
    }
#endif
    output << "\"];\n";
    output << "  total -> tag" << index << ";\n";
}

}

void MemoryVisualizer::WriteJson(const Core::MemoryStatistics& stats, std::ostream& output) {
    output << "{\n"
           << "  \"totalBytes\": " << stats.totalBytes << ",\n"
           << "  \"allocationCount\": " << stats.allocationCount << ",\n"
           << "  \"tags\": [\n";

    for (std::size_t i = 0; i < stats.tags.size(); ++i) {
        WriteTagJson(stats.tags[i], output, i + 1 == stats.tags.size());
    }

    output << "  ]\n"
           << "}\n";
}

void MemoryVisualizer::WriteGraphviz(const Core::MemoryStatistics& stats, std::ostream& output) {
    output << "digraph MemoryTags {\n";
    output << "  graph [rankdir=LR];\n";
    output << "  node [shape=box];\n";
    output << "  total [shape=oval,label=\"Total\nbytes: " << stats.totalBytes
           << "\\nallocations: " << stats.allocationCount << "\"];\n";

    for (std::size_t index = 0; index < stats.tags.size(); ++index) {
        WriteTagGraphviz(stats.tags[index], output, index);
    }

    output << "}\n";
}

} // namespace Memory::Profiler
