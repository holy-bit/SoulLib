# SoulLib

![CI Status](https://github.com/holy-bit/SoulLib/workflows/SoulLib%20CI/badge.svg)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

SoulLib is a modular C++20 toolkit that bundles coroutine-aware task scheduling, asynchronous file IO, dual-transport networking, container abstractions, memory tracking, and diagnostics. It is designed to slot into larger engines while remaining consumable as a standalone utility library.

## Highlights

- **Async task scheduling** – `soul::async::TaskScheduler` and `soul::time::FrameScheduler` run coroutines across a worker pool with explicit dependency graphs.
- **Asynchronous file system** – `AsyncFileManager` + `ThreadPoolAsyncFileIO` deliver non-blocking binary/text reads and writes with optional encryption strategies.
- **Modern networking** – `NetworkManager` orchestrates UDP (low-latency) and TCP (reliable) transports with sequence/ack metadata, optional ACK flags, and per-channel UDP retransmissions configurable via the task scheduler.
- **Container suite** – SBO-enabled `SoulVector`, cache-friendly `SoulFlatMap`, and STL wrappers that feed allocation telemetry back to the registry.
- **Memory instrumentation** – `Memory::Core::MemoryManager` plus the `SoulLibMemoryViz` CLI export JSON snapshots for dashboards and CI.
- **Debug/logging stack** – structured channels, formatters, and macros (`DEBUG_LOG`, `DEBUG_WARNING`, `DEBUG_ERROR`).
- **Examples, tests, benchmarks** – ready-to-run demos, GoogleTest coverage, and Google Benchmark suites.

## Documentation

For architectural details, integration patterns, and troubleshooting tips, see [`docs/Architecture.md`](docs/Architecture.md).

## Build Instructions

Prerequisites: **CMake ≥ 3.20** and a **C++20** toolchain (MSVC 19.3x, Clang 14+, or GCC 11+).

```powershell
cmake -S . -B build -DSOULLIB_BUILD_TESTS=ON -DSOULLIB_BUILD_BENCHMARKS=ON -DSOULLIB_BUILD_TOOLS=ON
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
```

Available toggles:

- `SOULLIB_BUILD_TESTS` (default ON)
- `SOULLIB_BUILD_BENCHMARKS` (default ON)
- `SOULLIB_BUILD_TOOLS` (builds Memory Visualizer CLI, default ON)
- `SOULLIB_BUILD_DOCS` (generates Doxygen docs when available, default OFF)

Artifacts (`SoulLib.dll`, `SoulLib.lib`, `SoulLibTests.exe`, `SoulLibMemoryViz.exe`) are output to `build/bin/<Config>/`.

## Repository Layout

| Path | Purpose |
|------|---------|
| `include/` | Public headers grouped by subsystem (Async, containers, FileSystem, Memory, Networking, Time, Debug). |
| `src/` | Module implementations mirroring `include/`. |
| `examples/` | Standalone samples (e.g., `ContainerMemoryExample.cpp`). |
| `demo/` | Interactive console suite showcasing timers, memory analytics, async jobs, logging, and persistence. |
| `test/` | GoogleTest suites organised per subsystem. |
| `benchmarks/` | Google Benchmark microbenchmarks. |
| `tools/` | Developer tools (`MemoryVisualizer`). |
| `docs/` | Extended documentation (`Architecture.md`, roadmap). |

## Getting Started

The quickest smoke test wires logging and prints a greeting:

```cpp
#include "utility.h"
#include "debug/Debug.h"

int main() {
    DebugConfig cfg{LogLevel::Info, LogLevel::Info, true, false, "", false, ""};
    Debug::Init(cfg);
    printHello();
    Debug::Shutdown();
    return 0;
}
```

From there, explore:

- `examples/ContainerMemoryExample.cpp` for allocator tagging and SBO containers.
- `demo/main.cpp` for an interactive suite showcasing five subsystem scenarios.
- `tools/MemoryVisualizer` to export current allocation statistics.
- `tools/DagVisualizer` to graph frame scheduler dependencies.

## Documentation

- [Architecture Overview](docs/Architecture.md) — Core design principles and subsystem relationships
- [API Reference](docs/BUILD_DOCUMENTATION.md) — Generate HTML documentation with Doxygen
- [Release Notes v0.3.0](docs/RELEASE_NOTES_v0.3.0.md) — Latest changes and migration guide

## Contributing

Contributions are welcome! See [`CONTRIBUTING.md`](CONTRIBUTING.md) for code style, PR workflow, and maintainer expectations. Open an issue or discussion before starting major work.

## License

SoulLib is released under the [MIT License](LICENSE). See the LICENSE file for details.

Happy hacking!
