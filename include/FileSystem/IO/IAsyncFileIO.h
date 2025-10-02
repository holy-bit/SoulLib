#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <system_error>
#include <vector>

#include "Async/Task.h"
#include "FileSystem/IO/FileResult.h"

namespace soul::filesystem::io {

// ReadFileResult and WriteFileResult now defined in FileResult.h (C++20/23 compatible)

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
