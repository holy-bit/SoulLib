# C++23 Migration Checklist

This document captures the actionable steps for validating SoulLib under the optional `SOULLIB_ENABLE_CPP23` build flag and tracking the remaining migration work.

## Current Validation Status

- **Compiler toolchains**: Verified MSVC 19.29.30159 (Visual Studio 2019) builds successfully with `/std:c++latest` via `SOULLIB_ENABLE_CPP23=ON`.
- **Unit tests**: `ctest` (95 tests) passes under the C++23 configuration as of 2025-09-29.
- **Benchmarks and tooling**: Memory Visualizer and benchmark targets compile without warnings in the C++23 build.

```powershell
# Configure and build C++23 profile
cmake -S . -B build-cpp23 -DSOULLIB_ENABLE_CPP23=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cpp23 --config Debug
ctest --test-dir build-cpp23 --output-on-failure
```

## In-Flight Tasks

1. **API Modernisation**
   - Adopt `std::expected` for async/file-system result objects (replace custom `Result`). **Status**: in progress — wrapper types drafted for `ReadFileResult`/`WriteFileResult`; integration pending compilation validation under C++23 profile.
   - Evaluate `std::mdspan` for container/allocator stress paths after benchmarking.
   - Audit tooling binaries (`SoulLibMemoryViz`, `SoulLibDagViz`, demos) for use of `std::print` where appropriate.
2. **Module Prototyping**
   - Expand the existing `SoulLibMemoryModules` experiment to additional subsystems (Networking, Async).
   - Measure build times with partitioned modules vs. header-only delivery.
3. **CI Integration**
   - Extend pipelines to build `SOULLIB_ENABLE_CPP23=ON` alongside the default C++20 configuration. **Status**: ✅ implemented — `.github/workflows/ci.yml` now runs both profiles on Windows (MSVC), Ubuntu (GCC 11, Clang 14), with benchmark regression tracking.
   - Store benchmark deltas to detect regressions when toggling between standards. **Status**: ✅ integrated via `benchmark-action/github-action-benchmark`.
4. **Documentation**
   - Reflect the C++23 validation status in `docs/Architecture.md` and roadmap updates.
   - Add troubleshooting tips for older toolchains that lack full C++23 support.

## Next Check-In

- Re-run the C++23 build and test validations after integrating `std::expected` conversion.
- Confirm GCC 14 and Clang 18 parity once Windows/Linux CI nodes are provisioned.

## Troubleshooting Legacy Toolchains

- **MSVC < 19.34** – lacks several `std::expected`/`std::print` headers. Require VS 2022 or newer
   when flipping `SOULLIB_ENABLE_CPP23`.
- **Clang < 17** – enable libc++ with `-stdlib=libc++` to access `<expected>` and `<print>` while the
   libstdc++ implementation completes. Pair with `-fexperimental-library` when using LLVM 16.
- **GCC < 13** – does not ship `<print>` and has partial `<expected>` support. Keep the default
   C++20 profile or upgrade to GCC 14 for full coverage before enabling the experimental flag.
- Always clear the CMake cache after toggling `SOULLIB_ENABLE_CPP23` to avoid conflicting standard
   library artefacts between builds.
