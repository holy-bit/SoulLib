# SoulLib Demo Suite

This interactive console demo showcases several subsystems of **SoulLib** directly from `demo/main.cpp`. The program exposes a small menu so you can pick different scenarios and observe timing behaviour, tagged memory accounting, container performance, the async scheduler, and the debug/file-system toolchain side by side.

## Building

The demo is self-contained and links against the locally built SoulLib binaries. Generate and build it in Debug (adjust the generator if needed):

```powershell
cmake -S demo -B demo/build -DCMAKE_BUILD_TYPE=Debug
cmake --build demo/build --config Debug
```

> The build directory is ignored by source control; feel free to delete it to rebuild from scratch.

## Running

```powershell
& "demo/build/bin/Debug/main.exe"
```

When the menu appears, enter the number of the scenario you want to explore, or `Q` to exit. After every run press **Enter** to return to the menu.

> The demo expects interactive input from a console. If you need to script executions (for CI or automated tests), set the environment variable `SOULLIB_DEMO_AUTOMATION=1` before launching the executable so piped input is accepted.

## Scenarios

- **Planificador de tareas temporizadas** - Runs `TimeService`, `TimerManager`, and multiple `TimedTask` instances to demonstrate real-time scheduling. You can observe how the manager reports the next completion and logs when each workload finishes.
- **Analizador de memoria con allocators etiquetados** - Uses `MemoryRegistry`, `TaggedMemoryAllocator`, and `SoulVector` to allocate thousands of elements, then prints detailed deltas per memory tag before and after clearing/shrinking the containers.
- **Comparativa de rendimiento entre contenedores** - Fills a `std::vector` that uses `MemoryAllocator` side by side with a `SoulVector`, timing insertion and iteration while also reporting the raw bytes observed by the memory manager.
- **Programación asincrónica con TaskScheduler** - Dispatches CPU-intensive jobs through `soul::async::TaskScheduler`, comparing sequential and parallel execution while validating deterministic checksums for each workload.
- **Logging filtrado y persistencia JSON** - Reconfigures the debug manager to showcase log-level filtering and then uses `FileSystem::Core::FileManager` to persist and reload a JSON snapshot with zero external dependencies.

## Extending the demo

Each scenario lives in its own function inside `main.cpp`. Add more cases by following the same structure: capture the state you want to illustrate, run the workload, emit compact diagnostics, and wire the option into the menu. This keeps SoulLib examples discoverable for new contributors while preserving a clear separation between subsystems.
