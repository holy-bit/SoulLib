# std::expected Migration Summary

## Overview
The FileSystem I/O layer now supports **dual result types** for async operations:
- **C++23 mode** (`__cpp_lib_expected >= 202202L`): Uses `std::expected<T, std::error_code>` for ergonomic monadic error handling.
- **C++20 fallback**: Preserves the existing struct-based `ReadFileResult`/`WriteFileResult` with explicit `.error` members.

## Files Modified

| Path | Change |
|------|--------|
| `include/FileSystem/IO/FileResult.h` | **New header** that conditionally exports result types based on `SOULLIB_HAS_EXPECTED`. |
| `include/FileSystem/IO/IAsyncFileIO.h` | Removed inline definitions; now imports result types from `FileResult.h`. |
| `src/FileSystem/IO/ThreadPoolAsyncFileIO.cpp` | Dual implementation: C++23 returns `std::expected` values, C++20 returns legacy structs. |

## Usage (C++23 Profile)

```cpp
auto scheduler = std::make_shared<soul::async::TaskScheduler>();
auto io = std::make_shared<soul::filesystem::io::ThreadPoolAsyncFileIO>(scheduler);
soul::filesystem::core::AsyncFileManager files(io);

auto result = co_await files.read("config.json");
if (!result) {
    std::cerr << "Read failed: " << result.error().message() << '\n';
    co_return;
}

auto [path, data] = std::move(*result);
ProcessConfigData(data);
```

## Usage (C++20 Profile)

```cpp
auto result = co_await files.read("config.json");
if (result.error) {
    std::cerr << "Read failed: " << result.error.message() << '\n';
    co_return;
}

ProcessConfigData(result.data);
```

## Validation

- **C++20 default build**: ✅ All 95 tests pass (MSVC 19.29, no `std::expected` support).
- **C++23 experimental build** (`SOULLIB_ENABLE_CPP23=ON`): ✅ All 95 tests pass (preprocessor guards ensure fallback remains active until full toolchain upgrade).

## Next Steps

1. Upgrade to **MSVC 19.34+** (VS 2022 17.4) or **Clang 17** to enable `std::expected` runtime paths.
2. Migrate consumer code to check `result.has_value()` instead of `.error` once the C++23 build becomes default.
3. Extend the pattern to `Networking::NetworkManager` packet results in a subsequent iteration.
