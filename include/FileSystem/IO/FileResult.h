#pragma once

#include <filesystem>
#include <span>
#include <system_error>
#include <vector>

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#include <expected>
#define SOULLIB_HAS_EXPECTED 1
#else
#define SOULLIB_HAS_EXPECTED 0
#endif

namespace soul::filesystem::io {

#if SOULLIB_HAS_EXPECTED

/**
 * @brief Result type for asynchronous read operations (C++23 profile).
 * @details When `std::expected` is available, we unify the success/error path into a single type.
 *          On success, `data` holds the file contents; on failure, the expected carries the error code.
 */
using ReadFileResult = std::expected<std::pair<std::filesystem::path, std::vector<std::byte>>, std::error_code>;

/**
 * @brief Result type for asynchronous write operations (C++23 profile).
 */
using WriteFileResult = std::expected<std::filesystem::path, std::error_code>;

#else

/**
 * @brief Result payload returned after an asynchronous read operation (C++20 fallback).
 */
struct ReadFileResult {
    std::vector<std::byte> data;      ///< Raw file contents; empty on failure.
    std::filesystem::path path;       ///< Absolute path that was read.
    std::error_code error;            ///< Error code describing failure (if any).
};

/**
 * @brief Result payload returned after an asynchronous write operation (C++20 fallback).
 */
struct WriteFileResult {
    std::filesystem::path path; ///< Destination that was written.
    std::error_code error;      ///< Error code describing failure (if any).
};

#endif

} // namespace soul::filesystem::io
