#include "Async/Task.h"
#include "FileSystem/Core/FileManager.h"
#include "debug/Debug.h"
#include "time/TimerManager.h"
#include "time/TimedTask.h"
#include "time/TimeService.h"
#include "Memory/Core/MemoryAllocator.h"
#include "Memory/Core/MemoryRegistry.h"
#include "Memory/Core/MemoryStatistics.h"
#include "Memory/Core/TaggedMemoryAllocator.h"
#include "Memory/Profiler/MemoryProfiler.h"
#include "containers/Sequential/SoulVector.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace demo {

using Memory::Core::MemoryStatistics;
using Memory::Core::MemoryTag;
using Memory::Core::MemoryTagStats;

namespace {

std::string formatBytes(std::size_t bytes) {
    static constexpr std::array<const char*, 5> kUnits{"B", "KiB", "MiB", "GiB", "TiB"};
    double value = static_cast<double>(bytes);
    std::size_t unitIndex = 0;
    while (value >= 1024.0 && unitIndex + 1 < kUnits.size()) {
        value /= 1024.0;
        ++unitIndex;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(value < 10.0 ? 2 : 1) << value << ' ' << kUnits[unitIndex];
    return oss.str();
}

std::string formatDuration(std::chrono::microseconds span) {
    using namespace std::chrono;
    std::ostringstream oss;

    if (span >= seconds{1}) {
        double secondsValue = static_cast<double>(span.count()) / 1'000'000.0;
        oss << std::fixed << std::setprecision(secondsValue < 10.0 ? 2 : 1) << secondsValue << " s";
    } else if (span >= milliseconds{1}) {
        double msValue = static_cast<double>(span.count()) / 1'000.0;
        oss << std::fixed << std::setprecision(msValue < 10.0 ? 2 : 1) << msValue << " ms";
    } else {
        oss << span.count() << " us";
    }

    return oss.str();
}

std::uint64_t runCpuBoundWork(std::uint32_t iterations) {
    std::uint64_t accumulator = 0;
    for (std::uint32_t i = 1; i <= iterations; ++i) {
        accumulator += static_cast<std::uint64_t>(i) * 2654435761ull;
        accumulator ^= (accumulator >> 11);
        accumulator += (accumulator << 7) ^ static_cast<std::uint64_t>(i * 97u);
    }
    return accumulator;
}

std::string tagLabel(const MemoryTag& tag) {
#ifdef SOUL_DEBUG
    if (!tag.label.empty()) {
        return std::string(tag.label);
    }
#endif
    std::ostringstream oss;
    oss << "tag:0x" << std::hex << std::setw(8) << std::setfill('0') << tag.hash;
    return oss.str();
}

struct TagDiff {
    MemoryTag tag{};
    std::size_t afterBytes{0};
    std::size_t beforeBytes{0};
    std::size_t afterAllocations{0};
    std::size_t beforeAllocations{0};
};

void printMemoryDelta(const MemoryStatistics& before, const MemoryStatistics& after) {
    std::unordered_map<uint32_t, MemoryTagStats> beforeMap;
    beforeMap.reserve(before.tags.size());
    for (const auto& tag : before.tags) {
        beforeMap.emplace(tag.tag.hash, tag);
    }

    std::vector<TagDiff> diffs;
    diffs.reserve(before.tags.size() + after.tags.size());

    for (const auto& current : after.tags) {
        TagDiff diff{};
        diff.tag = current.tag;
        diff.afterBytes = current.bytes;
        diff.afterAllocations = current.allocations;

        if (auto it = beforeMap.find(current.tag.hash); it != beforeMap.end()) {
            diff.beforeBytes = it->second.bytes;
            diff.beforeAllocations = it->second.allocations;
            beforeMap.erase(it);
        }

        if (diff.afterBytes != diff.beforeBytes || diff.afterAllocations != diff.beforeAllocations) {
            diffs.emplace_back(diff);
        }
    }

    // Tags that disappeared entirely
    for (const auto& entry : beforeMap) {
        TagDiff diff{};
        diff.tag = entry.second.tag;
        diff.beforeBytes = entry.second.bytes;
        diff.beforeAllocations = entry.second.allocations;
        diffs.emplace_back(diff);
    }

    const auto totalBytesDiff = static_cast<long long>(after.totalBytes) - static_cast<long long>(before.totalBytes);
    const auto totalAllocDiff = static_cast<long long>(after.allocationCount) - static_cast<long long>(before.allocationCount);

    std::cout << "\n  Memoria total monitorizada: " << formatBytes(after.totalBytes)
              << " (" << after.allocationCount << " allocations, "
              << (totalBytesDiff >= 0 ? '+' : '-') << formatBytes(static_cast<std::size_t>(std::llabs(totalBytesDiff))) << ")";
    std::cout << " | delta allocations: " << (totalAllocDiff >= 0 ? '+' : '-') << std::llabs(totalAllocDiff) << '\n';

    if (diffs.empty()) {
        std::cout << "    Sin cambios en las etiquetas de memoria.\n";
        return;
    }

    std::sort(diffs.begin(), diffs.end(), [](const TagDiff& lhs, const TagDiff& rhs) {
        const auto lhsDiff = static_cast<long long>(lhs.afterBytes) - static_cast<long long>(lhs.beforeBytes);
        const auto rhsDiff = static_cast<long long>(rhs.afterBytes) - static_cast<long long>(rhs.beforeBytes);
        return lhsDiff > rhsDiff;
    });

    for (const auto& diff : diffs) {
        const auto bytesDelta = static_cast<long long>(diff.afterBytes) - static_cast<long long>(diff.beforeBytes);
        const auto allocationsDelta = static_cast<long long>(diff.afterAllocations) - static_cast<long long>(diff.beforeAllocations);

        std::cout << "    - " << tagLabel(diff.tag) << " -> "
                  << (bytesDelta >= 0 ? '+' : '-') << formatBytes(static_cast<std::size_t>(std::llabs(bytesDelta)))
                  << " (total: " << formatBytes(diff.afterBytes) << ", allocs: " << diff.afterAllocations
                  << ", delta allocs: " << (allocationsDelta >= 0 ? '+' : '-') << std::llabs(allocationsDelta) << ")\n";
    }
}

void printMemorySnapshot(const MemoryStatistics& stats) {
    std::cout << "\n  Memoria actual -> " << formatBytes(stats.totalBytes)
              << " en " << stats.allocationCount << " allocations\n";
    if (stats.tags.empty()) {
        std::cout << "    (sin etiquetas registradas)\n";
        return;
    }
    for (const auto& tag : stats.tags) {
        std::cout << "    - " << tagLabel(tag.tag) << ": " << formatBytes(tag.bytes)
                  << " (" << tag.allocations << " allocations)\n";
    }
}

template <typename Callable>
std::chrono::microseconds measureMicroseconds(Callable&& callable) {
    const auto start = std::chrono::steady_clock::now();
    callable();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
}

void runSchedulerDemo() {
    std::cout << "\n=== Demo 1 - Planificador de tareas temporizadas ===\n";

    TimeService timeService;
    TimerManager timerManager;

    const auto start = std::chrono::steady_clock::now();
    const auto serverTime = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
    timeService.syncWithServerTime(static_cast<uint64_t>(serverTime));

    struct TaskSpec {
        std::string id;
        TimedTask::Priority priority;
        std::chrono::milliseconds offset;
        std::chrono::milliseconds duration;
    };

    const std::array<TaskSpec, 3> tasksConfig{{
        {"warmup", TimedTask::Priority::LOW, std::chrono::milliseconds{0}, std::chrono::milliseconds{1200}},
        {"streaming", TimedTask::Priority::MEDIUM, std::chrono::milliseconds{200}, std::chrono::milliseconds{3200}},
        {"critical", TimedTask::Priority::HIGH, std::chrono::milliseconds{500}, std::chrono::milliseconds{4800}}
    }};

    std::unordered_set<std::string> completed;
    std::vector<std::shared_ptr<TimedTask>> scheduled;
    scheduled.reserve(tasksConfig.size());

    for (const auto& spec : tasksConfig) {
        const uint64_t startTick = timeService.getCurrentTime() + static_cast<uint64_t>(spec.offset.count());
        const uint64_t endTick = startTick + static_cast<uint64_t>(spec.duration.count());
        auto task = std::make_shared<TimedTask>(startTick, endTick, spec.priority, spec.id);
        timerManager.addTask(task);
        scheduled.emplace_back(task);

        std::cout << "  - Programada '" << spec.id << "' (" << spec.priority << ") en t+"
                  << spec.offset.count() << " ms, duración " << spec.duration.count() << " ms\n";
    }

    std::cout << "\n  Simulando loop en tiempo real...\n";
    const auto pollInterval = std::chrono::milliseconds(150);

    while (!timerManager.getActiveTasks().empty()) {
        std::this_thread::sleep_for(pollInterval);
        timeService.updateRealTime();

        const uint64_t now = timeService.getCurrentTime();
        timerManager.updateAll(now);

        if (auto next = timerManager.getNextToComplete(now)) {
            const auto remaining = next->getRemainingTime(now);
            std::cout << "    Tick t=" << now << " ms -> próxima finalización: '" << next->getId()
                      << "' en " << remaining << " ms\n";
        }

        auto completedNow = timerManager.getCompletedTasks(now);
        for (const auto& task : completedNow) {
            if (completed.insert(task->getId()).second) {
                const auto duration = task->getEndTime() - task->getStartTime();
                std::cout << "      [done] Tarea '" << task->getId() << "' completada ("
                          << task->getPriority() << ", duración real " << duration << " ms)\n";
            }
        }
        timerManager.removeCompletedTasks(now);
    }

    std::cout << "  Todas las tareas han finalizado.\n";
}

void runAllocatorDemo() {
    std::cout << "\n=== Demo 2 - Analizador de memoria con allocators etiquetados ===\n";

    auto& registry = Memory::Core::MemoryRegistry::Get();
    const auto baseline = registry.snapshot();

    constexpr std::size_t sampleSize = 50'000;

    struct DemoVectorTag {
        static constexpr MemoryTag value() { return SOUL_MEMORY_TAG("DemoVector"); }
    };

    using ManagedAllocator = Memory::Core::TaggedMemoryAllocator<int, DemoVectorTag>;
    using ManagedStdVector = std::vector<int, ManagedAllocator>;

    ManagedStdVector tracked(ManagedAllocator{});
    ContainerSystem::Sequential::SoulVector<int> soulVector;

    tracked.reserve(sampleSize);
    soulVector.reserve(sampleSize);

    const auto fillDuration = measureMicroseconds([&]() {
        for (std::size_t i = 0; i < sampleSize; ++i) {
            tracked.emplace_back(static_cast<int>(i % 97));
        }
    });

    const auto soulFillDuration = measureMicroseconds([&]() {
        for (std::size_t i = 0; i < sampleSize; ++i) {
            soulVector.insert(static_cast<int>(i % 89));
        }
    });

    const auto afterFill = registry.snapshot();

    std::cout << "  Cargamos " << sampleSize << " elementos en ambos contenedores.\n";
    std::cout << "    std::vector<int> + TaggedAllocator -> " << formatDuration(fillDuration) << '\n';
    std::cout << "    SoulVector<int> -> " << formatDuration(soulFillDuration) << '\n';

    if (!tracked.empty()) {
        const auto rawBytes = Memory::Profiler::MemoryProfiler::measureRaw(tracked.data());
        std::cout << "    Reserva activa std::vector -> " << formatBytes(rawBytes) << '\n';
    }
    if (soulVector.size() > 16) { // mayor que small storage
        const auto rawBytes = Memory::Profiler::MemoryProfiler::measureRaw(soulVector.data());
        std::cout << "    Reserva activa SoulVector -> " << formatBytes(rawBytes) << '\n';
    }

    printMemoryDelta(baseline, afterFill);

    tracked.clear();
    soulVector.clear();
    const auto afterClear = registry.snapshot();

    std::cout << "\n  Tras clear():\n";
    printMemoryDelta(afterFill, afterClear);

    tracked.shrink_to_fit();
    soulVector.shrink_to_fit();
    const auto afterShrink = registry.snapshot();

    std::cout << "\n  Tras shrink_to_fit():\n";
    printMemoryDelta(afterClear, afterShrink);
}

void runContainerBenchmarkDemo() {
    std::cout << "\n=== Demo 3 - Comparativa de rendimiento entre contenedores ===\n";

    auto& registry = Memory::Core::MemoryRegistry::Get();
    const auto baseline = registry.snapshot();

    constexpr std::size_t sampleSize = 200'000;

    using ManagedStdVector = std::vector<int, Memory::Core::MemoryAllocator<int>>;
    ManagedStdVector stdVector;
    ContainerSystem::Sequential::SoulVector<int> soulVector;

    const auto stdFill = measureMicroseconds([&]() {
        stdVector.reserve(sampleSize);
        for (std::size_t i = 0; i < sampleSize; ++i) {
            stdVector.push_back(static_cast<int>(i));
        }
    });

    const auto soulFill = measureMicroseconds([&]() {
        soulVector.reserve(sampleSize);
        for (std::size_t i = 0; i < sampleSize; ++i) {
            soulVector.insert(static_cast<int>(i));
        }
    });

    const auto stdIteration = measureMicroseconds([&]() {
        std::uint64_t acc = 0;
        for (int value : stdVector) {
            acc += static_cast<std::uint64_t>(value);
        }
        volatile std::uint64_t sink = acc;
        (void)sink;
    });

    const auto soulIteration = measureMicroseconds([&]() {
        std::uint64_t acc = 0;
        for (int value : soulVector) {
            acc += static_cast<std::uint64_t>(value);
        }
        volatile std::uint64_t sink = acc;
        (void)sink;
    });

    const auto afterWork = registry.snapshot();

    std::cout << "  Inserción std::vector -> " << formatDuration(stdFill) << '\n';
    std::cout << "  Inserción SoulVector -> " << formatDuration(soulFill) << '\n';
    std::cout << "  Iteración std::vector -> " << formatDuration(stdIteration) << '\n';
    std::cout << "  Iteración SoulVector -> " << formatDuration(soulIteration) << '\n';

    if (!stdVector.empty()) {
        std::cout << "  Memoria std::vector (tag MemoryAllocator) -> "
                  << formatBytes(Memory::Profiler::MemoryProfiler::measureRaw(stdVector.data()))
                  << " | capacidad " << stdVector.capacity() << '\n';
    }
    if (soulVector.size() > 16) {
        std::cout << "  Memoria SoulVector (tag SoulVector) -> "
                  << formatBytes(Memory::Profiler::MemoryProfiler::measureRaw(soulVector.data()))
                  << " | capacidad " << soulVector.end() - soulVector.begin() << '\n';
    }

    printMemoryDelta(baseline, afterWork);

    stdVector.clear();
    stdVector.shrink_to_fit();
    soulVector.clear();
    soulVector.shrink_to_fit();

    const auto afterCleanup = registry.snapshot();
    std::cout << "\n  Tras liberar memoria de los contenedores:\n";
    printMemoryDelta(afterWork, afterCleanup);
}

void runAsyncJobSystemDemo() {
    std::cout << "\n=== Demo 4 - Programación de tareas asincrónicas ===\n";

    constexpr std::array<std::uint32_t, 4> workloads{160'000u, 170'000u, 180'000u, 200'000u};

    std::vector<std::uint64_t> sequentialResults;
    sequentialResults.reserve(workloads.size());

    const auto sequentialDuration = measureMicroseconds([&]() {
        for (auto iterations : workloads) {
            sequentialResults.push_back(runCpuBoundWork(iterations));
        }
    });

    soul::async::TaskScheduler scheduler(workloads.size());
    scheduler.run();
    struct SchedulerGuard {
        soul::async::TaskScheduler& scheduler;
        ~SchedulerGuard() { scheduler.stop(); }
    } guard{scheduler};

    std::vector<soul::async::Task<std::uint64_t>> tasks;
    tasks.reserve(workloads.size());
    std::vector<soul::async::TaskToken> tokens;
    tokens.reserve(workloads.size());

    const auto asyncDuration = measureMicroseconds([&]() {
        for (auto iterations : workloads) {
            auto task = scheduler.run_async([iterations]() -> std::uint64_t {
                return runCpuBoundWork(iterations);
            });
            tokens.emplace_back(task.token());
            tasks.emplace_back(std::move(task));
        }

        for (const auto& token : tokens) {
            scheduler.wait(token);
        }
    });

    std::vector<std::uint64_t> asyncResults;
    asyncResults.reserve(workloads.size());
    for (auto& task : tasks) {
        asyncResults.emplace_back(task.get());
    }

    std::cout << "  Trabajo secuencial -> " << formatDuration(sequentialDuration) << '\n';
    std::cout << "  Trabajo paralelo (" << workloads.size() << " workers) -> "
              << formatDuration(asyncDuration) << '\n';

    for (std::size_t i = 0; i < workloads.size(); ++i) {
        const bool match = sequentialResults[i] == asyncResults[i];
        std::cout << "    Lote " << (i + 1) << " (" << workloads[i] << " iteraciones) -> checksum 0x"
                  << std::hex << asyncResults[i] << std::dec
                  << (match ? " [OK]" : " [MISMATCH]") << '\n';
    }

    const auto checksum = std::accumulate(asyncResults.begin(), asyncResults.end(), std::uint64_t{0},
                                          [](std::uint64_t acc, std::uint64_t value) {
                                              return acc ^ (value + 0x9e3779b97f4a7c15ull + (acc << 6) + (acc >> 2));
                                          });

    std::cout << "  Hash combinada de resultados: 0x" << std::hex << checksum << std::dec << '\n';
}

void runLoggingAndPersistenceDemo() {
    std::cout << "\n=== Demo 5 - Logging filtrado y persistencia JSON ligera ===\n";

    DebugConfig filteredConfig{
        LogLevel::Info,
        LogLevel::Warning,
        true,
        false,
        {},
        false,
        {}
    };

    Debug::Init(filteredConfig);
    DEBUG_LOG("Este mensaje Info no se mostrará porque minLogLevel=Warning");
    DEBUG_WARNING("Advertencia inicial con filtrado activo.");
    DEBUG_ERROR("Error simulado para ilustrar el canal de alta prioridad.");
    Debug::Shutdown();

    DebugConfig verboseConfig{
        LogLevel::Debug,
        LogLevel::Debug,
        true,
        false,
        {},
        false,
        {}
    };

    Debug::Init(verboseConfig);
    DEBUG_LOG("Logging a nivel Debug activo; guardaremos un snapshot JSON temporal.");

    auto& fileManager = FileSystem::Core::FileManager::instance();
    const auto tempDir = std::filesystem::temp_directory_path();
    const auto filePath = tempDir / "soullib_demo_snapshot.json";
    const std::string pathString = filePath.string();

    if (fileManager.exists(pathString)) {
        fileManager.remove(pathString);
    }

    std::ostringstream payload;
    const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    payload << "{\n"
            << "  \"timestamp\": " << timestamp << ",\n"
            << "  \"diagnostics\": \"SoulLib puede persistir blobs JSON sin dependencias\",\n"
            << "  \"notes\": [\"Logging filtrado\", \"Persistencia JSON\"]\n"
            << "}\n";

    if (fileManager.saveJson(pathString, payload.str())) {
        DEBUG_LOG("Archivo JSON guardado en: " + pathString);
        const std::string restored = fileManager.loadJson(pathString);
        std::cout << "  JSON leído desde disco:\n" << restored << '\n';
        DEBUG_LOG("Contenido restaurado (" + std::to_string(restored.size()) + " bytes).");
        if (fileManager.remove(pathString)) {
            DEBUG_LOG("Archivo temporal limpiado correctamente.");
        } else {
            DEBUG_WARNING("No se pudo eliminar el archivo temporal. Revisa permisos.");
        }
    } else {
        DEBUG_ERROR("No se pudo escribir el archivo temporal: " + pathString);
    }

    Debug::Shutdown();

    std::cout << "  Revisa las trazas anteriores para observar el filtrado de niveles y la persistencia en disco.\n";
}

} // namespace

} // namespace demo

namespace {

void printMenu() {
    std::cout << "\n============================================\n";
    std::cout << "SoulLib Demo Suite\n";
    std::cout << "============================================\n";
    std::cout << "[1] Planificador de tareas temporizadas\n";
    std::cout << "[2] Analizador de memoria con allocators\n";
    std::cout << "[3] Comparativa de contenedores\n";
    std::cout << "[4] Programación asincrónica con TaskScheduler\n";
    std::cout << "[5] Logging y persistencia con FileManager\n";
    std::cout << "[Q] Salir\n";
    std::cout << "Seleccione una opción y presione Enter: " << std::flush;
}

void waitForEnter() {
    std::cout << "\nPresiona Enter para volver al menú principal...";
    std::string line;
    std::getline(std::cin, line);
}

} // namespace

namespace {

bool isInteractiveInput() {
#if defined(_WIN32)
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(fileno(stdin)) != 0;
#endif
}

bool automationOverrideEnabled() {
    return std::getenv("SOULLIB_DEMO_AUTOMATION") != nullptr;
}

} // namespace

int main() {
    const bool interactiveSession = isInteractiveInput();
    const bool automationOptIn = automationOverrideEnabled();

    if (!interactiveSession && !automationOptIn) {
        std::cout << "Este demo requiere una consola interactiva."
                  << " Ejecuta main.exe desde una terminal para seleccionar las demos.\n";
        return 0;
    }

    std::cout << "Bienvenido a la SoulLib Demo Suite" << std::endl;

    bool running = true;
    while (running) {
        printMenu();

        std::string input;
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input.empty()) {
            continue;
        }

        if (input == "q" || input == "Q") {
            running = false;
            continue;
        }

        int option = -1;
        try {
            option = std::stoi(input);
        } catch (const std::exception&) {
            std::cout << "Opción inválida. Intenta nuevamente.\n";
            continue;
        }

        switch (option) {
            case 1:
                demo::runSchedulerDemo();
                waitForEnter();
                break;
            case 2:
                demo::runAllocatorDemo();
                waitForEnter();
                break;
            case 3:
                demo::runContainerBenchmarkDemo();
                waitForEnter();
                break;
            case 4:
                demo::runAsyncJobSystemDemo();
                waitForEnter();
                break;
            case 5:
                demo::runLoggingAndPersistenceDemo();
                waitForEnter();
                break;
            default:
                std::cout << "Opción no reconocida: " << option << "\n";
                break;
        }
    }

    const auto finalStats = Memory::Core::MemoryRegistry::Get().snapshot();
    std::cout << "\n=== Resumen final de memoria ===\n";
    demo::printMemorySnapshot(finalStats);
    std::cout << "Hasta la próxima.\n";

    return 0;
}