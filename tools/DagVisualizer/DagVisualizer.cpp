#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Async/Task.h"
#include "time/FrameScheduler.h"

namespace soul::tools {

/**
 * @brief Simple utility that exports FrameScheduler task dependencies as Graphviz DOT.
 * @details This prototype demonstrates how token relationships can be traced to visualize
 *          execution DAGs. In a production setting, the scheduler would expose snapshot APIs
 *          for live instrumentation; for now we accept a set of (name, token, dependencies) tuples.
 */
class DagVisualizer {
public:
    struct TaskNode {
        std::string name;
        soul::async::TaskToken token;
        std::vector<soul::async::TaskToken> dependencies;
    };

    void add_node(std::string name, soul::async::TaskToken token,
                  std::vector<soul::async::TaskToken> dependencies = {}) {
        m_nodes.push_back({std::move(name), std::move(token), std::move(dependencies)});
    }

    void write_dot(std::ostream& out) const {
        out << "digraph FrameSchedulerDAG {\n";
        out << "  rankdir=TB;\n";
        out << "  node [shape=box, style=filled, fillcolor=lightblue];\n\n";

        // Build reverse lookup: token -> name
        std::unordered_map<const void*, std::string> tokenToName;
        for (const auto& node : m_nodes) {
            if (node.token.valid()) {
                tokenToName[node.token.state().get()] = node.name;
            }
        }

        // Emit nodes
        for (const auto& node : m_nodes) {
            out << "  \"" << node.name << "\";\n";
        }

        out << "\n";

        // Emit edges
        for (const auto& node : m_nodes) {
            for (const auto& dep : node.dependencies) {
                if (!dep.valid()) continue;
                auto it = tokenToName.find(dep.state().get());
                if (it != tokenToName.end()) {
                    out << "  \"" << it->second << "\" -> \"" << node.name << "\";\n";
                }
            }
        }

        out << "}\n";
    }

    void write_dot_file(const std::string& path) const {
        std::ofstream file(path);
        if (!file) {
            throw std::runtime_error("Failed to open output file: " + path);
        }
        write_dot(file);
    }

private:
    std::vector<TaskNode> m_nodes;
};

} // namespace soul::tools

namespace {

void print_usage(const char* progName) {
    std::cout << "Usage: " << progName << " <output.dot>\n";
    std::cout << "  Generates a Graphviz DOT file visualizing a sample FrameScheduler DAG.\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string outputPath = argv[1];

    try {
        // Example scenario: Build a mock DAG with several interdependent tasks
        auto scheduler = std::make_shared<soul::async::TaskScheduler>();
        scheduler->run();

        soul::time::FrameScheduler frame(scheduler);
        soul::tools::DagVisualizer visualizer;

        // Task A: independent
        auto handleA = frame.schedule("LoadTextures", scheduler->run_async([]() {
            std::cout << "[Task A] Loading textures...\n";
        }));
        visualizer.add_node("LoadTextures", handleA.token, {});

        // Task B: independent
        auto handleB = frame.schedule("LoadModels", scheduler->run_async([]() {
            std::cout << "[Task B] Loading models...\n";
        }));
        visualizer.add_node("LoadModels", handleB.token, {});

        // Task C: depends on A and B
        std::vector<soul::async::TaskToken> depsC = {handleA.token, handleB.token};
        auto handleC = frame.schedule("BuildMaterials",
            scheduler->run_async([]() {
                std::cout << "[Task C] Building materials from textures + models...\n";
            }),
            depsC
        );
        visualizer.add_node("BuildMaterials", handleC.token, depsC);

        // Task D: depends on C
        std::vector<soul::async::TaskToken> depsD = {handleC.token};
        auto handleD = frame.schedule("UploadToGPU",
            scheduler->run_async([]() {
                std::cout << "[Task D] Uploading resources to GPU...\n";
            }),
            depsD
        );
        visualizer.add_node("UploadToGPU", handleD.token, depsD);

        // Task E: parallel to the above chain
        auto handleE = frame.schedule("InitializeAudio", scheduler->run_async([]() {
            std::cout << "[Task E] Initializing audio subsystem...\n";
        }));
        visualizer.add_node("InitializeAudio", handleE.token, {});

        frame.wait_for_all();
        scheduler->stop();

        visualizer.write_dot_file(outputPath);
        std::cout << "DAG exported to: " << outputPath << "\n";
        std::cout << "Render with: dot -Tpng " << outputPath << " -o dag.png\n";

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
