# Contributor's Badge of Honor 🏆

## Community Guidelines

Welcome to SoulLib! This project thrives on clear separation of concerns: **SoulLib** provides domain-agnostic systems utilities, while consuming projects (like game engines) assemble higher-level features using the public API.

### Code Style

- **C++ Standard**: C++20 baseline; experimental C++23 profile available via `SOULLIB_ENABLE_CPP23=ON`.
- **Formatting**: Follow the existing patterns—4-space indentation, `CamelCase` for types, `snake_case` for variables/functions.
- **Documentation**: All public APIs require Doxygen-style `@brief`/`@param`/`@return` comments. See `include/Async/Task.h` for examples.
- **No External Dependencies**: SoulLib ships zero dependencies (beyond STL). Keep it that way.

### Pull Request Workflow

1. **Fork & Branch**: Create a feature branch from `main` (e.g., `feature/my-enhancement`).
2. **Write Tests**: All new functionality must include GoogleTest coverage in `test/`.
3. **Run Local CI**:
   ```powershell
   cmake -S . -B build -DSOULLIB_BUILD_TESTS=ON -DSOULLIB_BUILD_BENCHMARKS=ON
   cmake --build build --config Debug
   ctest --test-dir build --output-on-failure
   ```
4. **Document Changes**: Update `docs/Architecture.md` and roadmap if introducing new subsystems.
5. **Submit PR**: Describe the motivation, design trade-offs, and link to any related issues.

### What We're Looking For

- **Performance optimizations** with benchmark evidence (use `benchmarks/` suite).
- **Cross-platform fixes** for GCC/Clang (currently MSVC-focused).
- **Tooling enhancements** (memory profilers, visualizers, static analyzers).
- **Documentation improvements** (tutorials, troubleshooting tips, API clarifications).

### What We're Not Looking For

- Game-specific logic (that belongs in consumer projects, not SoulLib).
- Breaking changes to public APIs without RFC discussion.
- Dependencies on third-party libraries (even header-only ones).

### Code Review Expectations

- Maintainers aim for **24-48h** initial response time.
- Expect constructive feedback focused on maintainability, performance, and API ergonomics.
- Large contributions may require iterative design discussion before code approval.

### Recognition

Contributors who land merged PRs earn commit credit and are listed in the project's acknowledgments. Sustained contributors may be invited to join the maintainer team.

---

**Questions?** Open a discussion in [GitHub Discussions](https://github.com/holy-bit/SoulLib/discussions) or tag `@holy-bit` in an issue.
