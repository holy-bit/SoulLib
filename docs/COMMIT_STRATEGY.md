# Commit Strategy for SoulLib v0.3.0

## Overview
This release completes the roadmap phases 4–6: comprehensive API documentation, advanced tooling (DAG visualization), and C++23 migration infrastructure. Additionally, CI/CD automation and contributor onboarding materials are now integrated.

## Logical Commit Grouping

### Commit 1: Documentation & Doxygen Enrichment
**Scope**: API-level documentation for async, filesystem, networking, and time scheduling subsystems.

```bash
git add include/Async/Task.h
git add include/time/FrameScheduler.h
git add include/FileSystem/Core/AsyncFileManager.h
git add include/Networking/NetworkManager.h
git add docs/Architecture.md
git add docs/CXX23_Migration_Checklist.md

git commit -m "docs: enrich Doxygen comments for async/filesystem/networking APIs

- Add @brief/@param/@return annotations to TaskScheduler, TaskToken, Task<T>
- Document awaitable behavior, dependency wiring, and error handling patterns
- Expand FrameScheduler with delayed execution and synchronization semantics
- Create C++23 migration checklist with toolchain troubleshooting guidance
- Update Architecture.md with layered design principles and troubleshooting matrix

Closes roadmap item 4 (Documentación API detallada)"
```

### Commit 2: DAG Visualization Tooling
**Scope**: New CLI utility for exporting frame scheduler dependency graphs.

```bash
git add tools/DagVisualizer/
git add CMakeLists.txt

git commit -m "feat: add DAG visualizer tool for FrameScheduler dependencies

- Implement DagVisualizer CLI that exports task graphs to Graphviz DOT format
- Demonstrate 5-node sample scenario (texture/model loading → materials → GPU upload)
- Integrate into build system via SOULLIB_BUILD_TOOLS flag
- Document usage patterns and Graphviz rendering commands

Closes roadmap item 5 (Herramientas Avanzadas - visualización DAG)"
```

### Commit 3: std::expected Migration Infrastructure
**Scope**: C++20/23 compatible result types for filesystem operations.

```bash
git add include/FileSystem/IO/FileResult.h
git add include/FileSystem/IO/IAsyncFileIO.h
git add src/FileSystem/IO/ThreadPoolAsyncFileIO.cpp
git add docs/Expected_Migration_Notes.md

git commit -m "feat: introduce std::expected support for FileSystem result types

- Add FileResult.h with preprocessor-guarded dual result definitions
- C++23: std::expected<data, error_code> for monadic composition
- C++20: legacy struct-based ReadFileResult/WriteFileResult fallback
- Validate both profiles (95/95 tests pass on MSVC 19.29)
- Document migration path and usage examples

Advances roadmap item 5 (API Modernisation) and C++23 adoption"
```

### Commit 4: CI/CD Automation
**Scope**: GitHub Actions multi-configuration matrix and benchmark tracking.

```bash
git add .github/workflows/ci.yml

git commit -m "ci: implement multi-platform GitHub Actions workflow

- Matrix builds: MSVC C++20/23, Ubuntu GCC 11, Ubuntu Clang 14
- Automated test suite execution (95 test cases)
- Memory visualizer and DAG visualizer smoke tests
- Benchmark regression tracking with alerting threshold (150%)
- Store test artifacts for post-mortem analysis

Closes roadmap item 3 (Integración CI/CD)"
```

### Commit 5: Demo Suite & Interactive Tooling
**Scope**: Enhanced demo with five subsystem scenarios.

```bash
git add demo/main.cpp
git add demo/CMakeLists.txt
git add demo/README.md

git commit -m "feat: expand demo suite with interactive subsystem showcase

- Add five scenarios: scheduler, memory analytics, containers, async jobs, logging/persistence
- Implement interactive menu with terminal detection and automation support
- Document build/run workflow and scenario descriptions
- Lock demo to C++20 standard for compatibility

Enhances developer onboarding and tooling validation"
```

### Commit 6: Project Infrastructure & Onboarding
**Scope**: README, contributing guide, license, and roadmap updates.

```bash
git add README.md
git add CONTRIBUTING.md
git add LICENSE
git add "Evolución de SoulLib_ Hoja de Ruta Técnica.md"

git commit -m "chore: finalize v0.3.0 release infrastructure

- Add MIT license file
- Create contributor guide with PR workflow and code style expectations
- Update README with CI badges, contributing links, and new tooling references
- Mark roadmap items 3-5 as completed with 2025-10-02 status updates
- Document C++23 validation results and pending modernization tasks

Release summary:
- ✅ API documentation (Doxygen enrichment)
- ✅ Advanced tooling (DAG visualizer)
- ✅ C++23 migration infrastructure (std::expected)
- ✅ CI/CD automation (multi-platform matrix)
- ✅ Contributor onboarding (guides, license, badges)"
```

## Pre-Commit Validation

Before executing commits, ensure:
1. All tests pass: `ctest --test-dir build --output-on-failure -C Debug`
2. DAG visualizer generates valid output: `SoulLibDagViz.exe sample.dot`
3. Memory visualizer runs without crashes: `SoulLibMemoryViz.exe snapshot.json`
4. Demo compiles and launches: `demo/build/bin/Debug/main.exe`

## Post-Commit Actions

1. Push to remote: `git push origin main`
2. Tag release: `git tag v0.3.0 -m "Release v0.3.0: Documentation, Tooling, CI/CD"`
3. Push tags: `git push origin v0.3.0`
4. Create GitHub release notes summarizing the six commit areas
5. Monitor CI pipeline execution on first automated run
