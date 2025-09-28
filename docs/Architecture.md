# SoulLib Architecture & Usage Guide

SoulLib is a modular C++ utility library that now ships with coroutine-friendly task execution, asynchronous file IO, modernised networking, and comprehensive memory tooling. It is intended to be embedded inside engines such as **SoulBox**, yet each subsystem can be consumed independently for games, simulations, and tooling projects.

---

## 1. Library at a Glance

| Subsystem | Purpose | Key Entry Points |
|-----------|---------|------------------|
| Async & Scheduling | Worker-backed task execution, coroutine wrappers, frame-aware dependency graphs | `Async/Task.h`, `time/FrameScheduler.h` |
| Memory Core | Allocation registry, pool/arena allocators, per-tag instrumentation | `Memory/Core/MemoryManager.h`, `Memory/Core/MemoryRegistry.h` |
| Memory Tooling | JSON export of allocation data for dashboards & CI | `Memory/Profiler/MemoryVisualizer.h`, tool `tools/MemoryVisualizer/` |
| Containers | STL-compatible containers with tagged allocators & SBO optimisations | `include/containers/ContainerSystem.h`, `Sequential::SoulVector`, `Associative::SoulFlatMap` |
| FileSystem | High-level async file facade with optional encryption | `FileSystem/Core/AsyncFileManager.h`, `FileSystem/IO/ThreadPoolAsyncFileIO.h` |
| Networking | Dual-transport (UDP/TCP) messaging with reliability metadata | `Networking/NetworkManager.h`, `Networking/UdpTransport.h`, `Networking/TcpTransport.h` |
| Debug & Diagnostics | Structured logging, formatting, and macros | `debug/Debug.h`, `debug/Formatter.h` |
| Time (legacy compatibility) | Timed tasks/events compatible with existing systems | `time/TimerManager.h`, `time/TimedTask.h` |
| Utility & Examples | Thin helpers and demos | `utility.h`, `demo/main.cpp`, `examples/ContainerMemoryExample.cpp` |

These modules are designed to compose. For example, containers and networking leverage the memory registry for tagged accounting, frame scheduling uses the async task scheduler, and the asynchronous file system executes heavy IO via the shared worker pool.

---

## 2. Build, Install, and Test

SoulLib requires **CMake ≥ 3.20** and a **C++20** compiler. The root project exports both shared and static targets (`Soul::SoulLib`, `Soul::SoulLibStatic`).

```powershell
# From C:\Users\luisc\Documents\Projects\SoulSystem\SoulLib
cmake -S . -B build -DSOULLIB_BUILD_TESTS=ON -DSOULLIB_BUILD_BENCHMARKS=ON -DSOULLIB_BUILD_TOOLS=ON
cmake --build build --config Debug
ctest --test-dir build --output-on-failure
```

Optional toggles:

* `SOULLIB_BUILD_TESTS` – GoogleTest suite (default ON).
* `SOULLIB_BUILD_BENCHMARKS` – Google Benchmark microbenchmarks (default ON).
* `SOULLIB_BUILD_TOOLS` – developer utilities (memory visualiser) (default ON).
* `SOULLIB_BUILD_DOCS` – generate Doxygen documentation when `doxygen` is available (default OFF).

Installation exports CMake package files so external projects can simply `find_package(SoulLib REQUIRED)` and link against `Soul::SoulLib`.

---

## 3. Bootstrapping & Initialisation Pattern

1. **Spin up debugging** – configure `DebugConfig`, call `Debug::Init`, and register extra channels if required.
2. **Attach a memory manager** – create `Memory::Core::MemoryManager`, enable debug mode if desired, and set it as the global registry (`MemoryRegistry::Set`).
3. **Create a shared task scheduler** – instantiate `std::make_shared<soul::async::TaskScheduler>()`. Reuse this across async file IO, frame scheduling, and network jobs.
4. **Configure file services** – build an `AsyncFileManager` with your scheduler and IO/encryption strategies. This handles both binary blobs and UTF-8 text asynchronously.
5. **Wire networking** – create UDP/TCP transports bound to sockets, inject them into `NetworkManager`, and start polling `receive()` inside the scheduler.
6. **Schedule work** – submit gameplay/system coroutines via `FrameScheduler` so dependencies and timed delays are handled automatically.
7. **Shutdown gracefully** – flush in-flight tasks (`wait_for_all`), close sockets, export optional memory snapshots, call `Debug::Shutdown`.

---

## 4. Subsystem Deep Dive

### 4.1 Async & Task Scheduling

* **`soul::async::TaskScheduler`** owns a worker pool (defaults to hardware concurrency) and accepts coroutine-based tasks.
  * `schedule(Task<T>, dependencies)` wires explicit DAG edges; tasks start when dependencies finish.
  * `run_async(callable)` executes blocking functions on workers and returns a `Task<T>` awaitable.
  * `Task<T>` objects can be `co_await`-ed or synchronously `get()`-ed; continuations resume on the scheduler.
* **`soul::time::FrameScheduler`** builds on the task scheduler to orchestrate frame-level jobs.
  * `schedule(name, task, dependencies)` registers a coroutine and returns a `TaskHandle` (with `TaskToken`).
  * `schedule_after(delay, name, task, dependencies)` introduces timed offsets using the shared scheduler.
  * `wait_for_all()` blocks until every scheduled task completes—a useful barrier for shutdown and tests.

```cpp
auto scheduler = std::make_shared<soul::async::TaskScheduler>();
soul::time::FrameScheduler frame{scheduler};

auto loadTextures = frame.schedule("textures", scheduler->run_async([] {
    return LoadTextureBlob("assets/player.dds");
}));

frame.schedule("materials", [&]() -> soul::async::Task<void> {
    auto textures = co_await loadTextures.task;
    BuildMaterials(textures);
    co_return;
}(), {loadTextures.token});

frame.wait_for_all();
```

### 4.2 Memory Core & Profiling

* **`MemoryManager`** records allocations tagged via constexpr hashes (`MemoryTag`). It now exposes a `snapshot()` API returning `MemoryStatistics` with totals and per-tag aggregates.
* **Pool & arena allocators** continue to serve fixed-size and transient workloads.
* **`MemoryVisualizer`** converts `MemoryStatistics` to JSON, enabling dashboards or CI checks. Run the CLI tool to capture the current registry state:

```powershell
# After your program executed allocations
SoulLibMemoryViz snapshot.json
```

Output example:

```json
{
  "totalBytes": 65536,
  "allocationCount": 42,
  "tags": [
    { "hash": 109499245, "label": "Render/Buffers", "bytes": 32768, "allocations": 3 },
    { "hash": 4018836991, "label": "AI/NavMesh", "bytes": 12000, "allocations": 12 }
  ]
}
```

### 4.3 Containers

* **`SoulVector<T, SmallCapacity>`** – header-only vector with configurable SBO; returning to the static buffer after `shrink_to_fit` releases heap memory.
* **`SoulFlatMap<K, V>`** – contiguous associative container (`std::vector` under the hood) with binary search lookups and tagged allocator support.
* Wrappers for STL containers still route allocations through `TaggedMemoryAllocator`, enabling memory reports and leak detection.
* Companion helpers (`ContainerMemoryUtils`) can optimise or audit memory usage for any registered container.

### 4.4 FileSystem

* **`AsyncFileManager`** couples an async IO backend with encryption strategies. The default backend, `ThreadPoolAsyncFileIO`, offloads blocking work to the shared scheduler and returns `Task` awaitables.
* Reads and writes deliver structured results (`ReadFileResult`, `WriteFileResult`) containing canonical paths and `std::error_code` diagnostics.
* Encryption remains pluggable (`NoEncryption`, `AesEncryption`). Provide custom strategies to integrate platform-specific crypto.

```cpp
auto scheduler = std::make_shared<soul::async::TaskScheduler>();
auto io = std::make_shared<soul::filesystem::io::ThreadPoolAsyncFileIO>(scheduler);
soul::filesystem::core::AsyncFileManager files(io);

co_await files.write("save.bin", std::as_bytes(std::span{buffer}));
auto result = co_await files.read("save.bin");
if (result.error) throw std::system_error(result.error);
```

### 4.5 Networking

* **`Networking::NetworkManager`** unifies UDP (low latency) and TCP (reliable/control) transports.
  * Packets carry headers with sequence numbers, ack masks, optional ACK flags, and delivery guarantees.
  * UDP uses contiguous header+payload framing; TCP sends a length-prefixed stream.
  * Reliability metadata is tracked per-channel; ordering guarantees can be implemented on top of sequence numbers.
  * `enable_udp_reliability(channel, true)` activates retransmissions over UDP using the built-in ACK window (32 packets) with tunable timeout/retry budget (`configure_udp_retransmission`).
* Transports run their socket calls inside `run_async` jobs, ensuring polling never blocks the main thread.
* Tests under `test/network/NetworkManagerTests.cpp` validate loopback UDP delivery and channel sequencing.

### 4.6 Time Services

* **`FrameScheduler`** is now the recommended orchestration primitive for gameplay-flow and simulation steps.
* Legacy components (`TimeService`, `TimerManager`, `TimedTask`, `EventScheduler`) remain available for existing code and examples; they can be progressively migrated by wrapping their callbacks in `FrameScheduler::schedule`.

### 4.7 Debug & Logging

* `Debug::Init` configures sinks, severity thresholds, and optional file logging.
* `DEBUG_LOG`/`DEBUG_WARNING`/`DEBUG_ERROR` macros include source information automatically.
* Custom channels are registered via `Debug::Manager::RegisterChannel`.

### 4.8 Utility & Examples

* `printHello()` still offers a quick smoke test.
* `examples/ContainerMemoryExample.cpp` demonstrates allocator tagging plus the new `MemoryVisualizer` to inspect allocations.
* `tools/MemoryVisualizer` and benchmarks live under `tools/` and `benchmarks/` respectively.

---

## 5. Example Scenarios

### 5.1 Asynchronous Asset Pipeline

```cpp
auto scheduler = std::make_shared<soul::async::TaskScheduler>();
soul::filesystem::core::AsyncFileManager files(std::make_shared<soul::filesystem::io::ThreadPoolAsyncFileIO>(scheduler));
soul::time::FrameScheduler frame(scheduler);

auto textures = frame.schedule("load_textures", [&]() -> soul::async::Task<std::vector<std::byte>> {
    auto data = co_await files.read("assets/atlas.bin");
    if (data.error) throw std::system_error(data.error);
    co_return std::move(data.data);
}());

frame.schedule("upload_textures", [&]() -> soul::async::Task<void> {
    auto atlas = co_await textures.task;
    UploadToGPU(atlas);
    co_return;
}(), {textures.token});

frame.wait_for_all();
```

### 5.2 Automated Memory Audits

```powershell
# Run your test harness first (allocations recorded by MemoryManager)
ctest --test-dir build --output-on-failure

# Export the current allocation snapshot
build/bin/Debug/SoulLibMemoryViz.exe build/reports/memory.json
```

Feed `memory.json` into dashboards or regression alerts to flag leaks or regressions per tag.

### 5.3 Low-Latency Messaging

```cpp
auto scheduler = std::make_shared<soul::async::TaskScheduler>();
auto udp = std::make_shared<soul::net::UdpTransport>(scheduler);
auto tcp = std::make_shared<soul::net::TcpTransport>(scheduler);
udp->bind(soul::net::Endpoint::from_string("0.0.0.0", 35000));

soul::net::NetworkManager net(tcp, udp);

soul::net::Packet packet;
packet.header.channel = 0;
packet.header.guarantee = soul::net::DeliveryGuarantee::Unreliable;
packet.payload.assign({std::byte{'P'}, std::byte{'I'}, std::byte{'N'}, std::byte{'G'}});

net.send(soul::net::Endpoint::from_string("127.0.0.1", 35001), std::move(packet)).get();
```

---

## 6. Testing Strategy

* `test/Async` (implicit via time tests) ensures task ordering and delayed scheduling semantics.
* `test/FileSystem/IO/AsyncFileManagerTests.cpp` validates async read/write round trips.
* `test/network/NetworkManagerTests.cpp` covers UDP channel sequencing.
* Existing suites for containers, memory, debug, and time continue to run via `SoulLibTests`.
* CI recommendation: run `ctest`, `SoulLibMemoryViz`, and (optionally) compare benchmark outputs for performance regressions.

---

## 7. Tooling & Documentation

* **Doxygen**: enable `SOULLIB_BUILD_DOCS` to generate HTML documentation in `build/docs/html`.
* **Memory Visualizer**: built when `SOULLIB_BUILD_TOOLS=ON`; outputs JSON for dashboards.
* **Benchmarks**: `benchmarks/` house Google Benchmark suites for allocators and containers.

---

## 8. Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| `co_await` never resumes | Task not scheduled on shared `TaskScheduler` | Use `FrameScheduler` or call `scheduler->schedule(std::move(task))` before awaiting |
| Async file read returns empty data | I/O error propagated via `ReadFileResult::error` | Inspect `error.message()`, ensure path exists, verify permissions |
| UDP packets not received | Missing `bind` on receiver or firewall | Bind transports to explicit ports, ensure firewall allows UDP |
| Memory snapshot empty | Memory manager not set or allocations already freed | Call `MemoryRegistry::Set(manager)` before allocations, export snapshot before teardown |
| Doxygen target fails | `SOULLIB_BUILD_DOCS=ON` but Doxygen not installed | Install Doxygen or disable docs option |

---

## 9. Further Reading

* [`docs/Architecture.md`](Architecture.md) – this document.
* [`README.md`](../README.md) – quick start & repo layout.
* [`examples/ContainerMemoryExample.cpp`](../examples/ContainerMemoryExample.cpp) – allocator tagging walkthrough.
* [`demo/main.cpp`](../demo/main.cpp) – demonstrates timers with async scheduling.
* [`tools/MemoryVisualizer/`](../tools/MemoryVisualizer) – source for the JSON exporter.
* [`benchmarks/`](../benchmarks) – allocator & container microbenchmarks.

Happy hacking—enjoy the upgraded async-first SoulLib!  
