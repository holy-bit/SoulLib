# Building SoulLib Documentation

This guide explains how to generate the complete HTML API reference using Doxygen.

---

## Prerequisites

### Windows (via winget)
```powershell
# Install Doxygen
winget install --id DimitriVanHeesch.Doxygen

# Install Graphviz (for dependency graphs)
winget install --id Graphviz.Graphviz

# Refresh PATH in current session
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
```

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y doxygen graphviz
```

### macOS (via Homebrew)
```bash
brew install doxygen graphviz
```

---

## Build Steps

### 1. Configure CMake with Documentation Enabled
```bash
cmake -B build -DSOULLIB_BUILD_DOCS=ON
```

This will:
- Detect Doxygen and Graphviz installations
- Create a `SoulLibDocs` build target
- Configure the `Doxyfile` from `docs/Doxyfile.in`

### 2. Generate HTML Documentation
```bash
# Build the documentation target
cmake --build build --target SoulLibDocs --config Release
```

**Output location**: `build/docs/html/`

### 3. Open in Browser
**Windows**:
```powershell
Start-Process build\docs\html\index.html
```

**Linux/macOS**:
```bash
xdg-open build/docs/html/index.html    # Linux
open build/docs/html/index.html         # macOS
```

---

## Documentation Structure

The generated documentation includes:

| Section | Description |
|---------|-------------|
| **Classes** | All SoulLib types (Task, TaskScheduler, FrameScheduler, etc.) |
| **Namespaces** | Organized by module (soul::async, soul::time, soul::fs, etc.) |
| **Files** | Header file index with inclusion graphs |
| **Modules** | Logical grouping (Memory, Async, FileSystem, Networking) |
| **Examples** | Code snippets from headers and `examples/` directory |

### Key Entry Points
- **Task Scheduling**: `soul::async::TaskScheduler`, `soul::async::Task<T>`
- **Frame Orchestration**: `soul::time::FrameScheduler`
- **Async File I/O**: `soul::fs::AsyncFileManager`, `soul::fs::IAsyncFileIO`
- **Networking**: `soul::net::NetworkManager`, `soul::net::Client`
- **Memory Management**: `soul::memory::MemoryManager`, `soul::memory::TaggedMemoryAllocator`

---

## Statistics (v0.3.0)

- **Total Files Generated**: 1,048 HTML pages
- **Classes Documented**: 50+ with full Doxygen annotations
- **Code Examples**: 15+ embedded in documentation
- **Dependency Graphs**: Enabled via Graphviz integration
- **Build Time**: ~10 seconds (Release mode, parallel build)

---

## Troubleshooting

### Issue: "Doxygen not found"
**Cause**: CMake cannot locate `doxygen` executable in PATH.

**Solution**:
1. Verify installation: `doxygen --version`
2. If not found, restart terminal or manually add to PATH:
   ```powershell
   # Windows
   $env:Path += ";C:\Program Files\doxygen\bin"
   ```

### Issue: "Graphviz dot not found"
**Cause**: Doxygen cannot generate dependency graphs without `dot` executable.

**Solution**:
1. Verify installation: `dot -V`
2. Add Graphviz bin directory to PATH:
   ```powershell
   # Windows
   $env:Path += ";C:\Program Files\Graphviz\bin"
   ```

### Issue: Warnings about missing README.md
**Cause**: Doxyfile references README.md as mainpage, but path resolution differs between systems.

**Impact**: Minimal — does not affect API reference quality.

**Solution**: Use absolute path in `docs/Doxyfile.in`:
```cmake
USE_MDFILE_AS_MAINPAGE = @CMAKE_SOURCE_DIR@/README.md
```

---

## CI/CD Integration

The `.github/workflows/ci.yml` workflow **does not** currently generate documentation as part of automated builds to reduce CI runtime. To publish docs:

### Option 1: GitHub Pages (Recommended)
```bash
# Generate docs locally
cmake -B build -DSOULLIB_BUILD_DOCS=ON
cmake --build build --target SoulLibDocs

# Copy to gh-pages branch
git worktree add gh-pages gh-pages
cp -r build/docs/html/* gh-pages/
cd gh-pages
git add .
git commit -m "docs: Update API reference for v0.3.0"
git push origin gh-pages
```

### Option 2: Dedicated Workflow
Create `.github/workflows/docs.yml` triggered on `main` branch pushes to auto-deploy to GitHub Pages.

---

## Additional Resources

- **Doxygen Manual**: https://www.doxygen.nl/manual/
- **Graphviz Documentation**: https://graphviz.org/documentation/
- **SoulLib Architecture**: [docs/Architecture.md](Architecture.md)
- **Contributing Guide**: [CONTRIBUTING.md](../CONTRIBUTING.md)

---

**Last Updated**: October 2, 2025 (v0.3.0)
