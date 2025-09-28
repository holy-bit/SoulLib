#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <vector>

#include "Async/Task.h"

namespace soul::filesystem::io {

/**
 * @brief Result payload returned after an asynchronous read operation.
 */
struct ReadFileResult {
    std::vector<std::byte> data;      ///< Raw file contents; empty on failure.
    std::filesystem::path path;       ///< Absolute path that was read.
    std::error_code error;            ///< Error code describing failure (if any).
};

/**
 * @brief Result payload returned after an asynchronous write operation.
 */
struct WriteFileResult {
    std::filesystem::path path; ///< Destination that was written.
    std::error_code error;      ///< Error code describing failure (if any).
};

/**
 * @brief Interface that exposes asynchronous file I/O operations.
 */
class IAsyncFileIO {
public:
    virtual ~IAsyncFileIO() = default;

    /**
     * @brief Asynchronously reads an entire file into memory.
     *
     * @param path Path to the file that should be read.
     * @return Task that completes with the read result.
     */
    virtual soul::async::Task<ReadFileResult> read(std::filesystem::path path) = 0;

    /**
     * @brief Asynchronously writes a buffer to disk, replacing the contents of the file.
     *
     * @param path Path to the file to write.
     * @param data Buffer to write; the data is copied before returning.
     * @return Task that completes with the write result.
     */
    virtual soul::async::Task<WriteFileResult> write(std::filesystem::path path,
                                                     std::span<const std::byte> data) = 0;
};

} // namespace soul::filesystem::io
