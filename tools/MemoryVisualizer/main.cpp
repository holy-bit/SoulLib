#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Memory/Core/MemoryRegistry.h"
#include "Memory/Profiler/MemoryVisualizer.h"

namespace {

enum class OutputFormat {
    Json,
    Graphviz
};

std::optional<std::ofstream> open_output_file(const std::string& path) {
    if (path.empty()) {
        return std::nullopt;
    }
    std::ofstream file(path, std::ios::trunc);
    if (!file) {
        throw std::runtime_error("Unable to open output file: " + path);
    }
    return file;
}

[[noreturn]] void show_usage_and_exit() {
    std::cout << "Usage: SoulLibMemoryViz [--format json|dot] [--output <path>]" << std::endl;
    std::exit(EXIT_SUCCESS);
}

struct CommandLineOptions {
    OutputFormat format{OutputFormat::Json};
    std::string outputPath;
};

CommandLineOptions parse_arguments(int argc, char** argv) {
    CommandLineOptions options;
    bool explicitOutput = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
            show_usage_and_exit();
        } else if (arg.rfind("--format=", 0) == 0) {
            const auto value = arg.substr(std::string_view("--format=").size());
            if (value == "json") {
                options.format = OutputFormat::Json;
            } else if (value == "dot") {
                options.format = OutputFormat::Graphviz;
            } else {
                throw std::invalid_argument("Unknown format: " + std::string(value));
            }
        } else if (arg == "--format") {
            if (++i >= argc) {
                throw std::invalid_argument("--format requires a value");
            }
            std::string_view value(argv[i]);
            if (value == "json") {
                options.format = OutputFormat::Json;
            } else if (value == "dot") {
                options.format = OutputFormat::Graphviz;
            } else {
                throw std::invalid_argument("Unknown format: " + std::string(value));
            }
        } else if (arg.rfind("--output=", 0) == 0) {
            options.outputPath = std::string(arg.substr(std::string_view("--output=").size()));
            explicitOutput = true;
        } else if (arg == "--output") {
            if (++i >= argc) {
                throw std::invalid_argument("--output requires a path");
            }
            options.outputPath = std::string(argv[i]);
            explicitOutput = true;
        } else if (!explicitOutput && options.outputPath.empty() && arg.front() != '-') {
            // Backward compatibility with positional path argument.
            options.outputPath = std::string(arg);
            explicitOutput = true;
        } else {
            throw std::invalid_argument("Unrecognized argument: " + std::string(arg));
        }
    }

    return options;
}

void write_statistics(const Memory::Core::MemoryStatistics& stats, std::ostream& output, OutputFormat format) {
    switch (format) {
    case OutputFormat::Json:
        Memory::Profiler::MemoryVisualizer::WriteJson(stats, output);
        break;
    case OutputFormat::Graphviz:
        Memory::Profiler::MemoryVisualizer::WriteGraphviz(stats, output);
        break;
    }
}

}

int main(int argc, char** argv) {
    try {
        const auto options = parse_arguments(argc, argv);

        auto& manager = Memory::Core::MemoryRegistry::Get();
        auto stats = manager.snapshot();

        if (auto file = open_output_file(options.outputPath); file.has_value()) {
            write_statistics(stats, *file, options.format);
        } else {
            write_statistics(stats, std::cout, options.format);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Memory visualization failed: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
