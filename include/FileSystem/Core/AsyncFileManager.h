#pragma once

#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "Async/Task.h"
#include "FileSystem/Core/IFileManager.h"
#include "FileSystem/Encryption/IEncryptionStrategy.h"
#include "FileSystem/Encryption/NoEncryption.h"
#include "FileSystem/IO/IAsyncFileIO.h"

namespace soul::filesystem::core {

namespace Encryption = ::FileSystem::Encryption;

/**
 * @brief High-level façade that combines asynchronous file I/O with optional encryption.
 * @details The manager wraps an `IAsyncFileIO` backend and an encryption strategy. All methods
 *          return awaitable tasks that dispatch the requested work to the shared async scheduler.
 *          The returned results always include the canonical path to simplify observability.
 */
class AsyncFileManager {
public:
    AsyncFileManager(std::shared_ptr<io::IAsyncFileIO> ioBackend,
                     std::unique_ptr<Encryption::IEncryptionStrategy> encryption =
                         std::make_unique<Encryption::NoEncryption>());

    AsyncFileManager(const AsyncFileManager&) = delete;
    AsyncFileManager& operator=(const AsyncFileManager&) = delete;

    AsyncFileManager(AsyncFileManager&&) noexcept = default;
    AsyncFileManager& operator=(AsyncFileManager&&) noexcept = default;

    /**
     * @brief Asynchronously reads the entire file into memory.
     * @param path Path to the file on disk.
     * @return Awaitable that resolves to a `ReadFileResult` with canonical path, buffer, and error
     *         code. The caller should inspect `error` before consuming `data`.
     */
    soul::async::Task<io::ReadFileResult> read(std::filesystem::path path);

    /**
     * @brief Asynchronously writes the provided data to the destination file.
     * @param path Target file path, parent directories must exist.
     * @param data Binary payload to encrypt (optional) and persist to disk.
     * @return Awaitable that resolves to a `WriteFileResult` capturing the canonical destination and
     *         any error encountered.
     */
    soul::async::Task<io::WriteFileResult> write(std::filesystem::path path,
                                                 std::span<const std::byte> data);

    /**
     * @brief Asynchronously loads UTF-8 text from disk.
     * @param path File path to read.
     * @return Awaitable returning a populated UTF-8 string. Errors surface as exceptions so the
     *         coroutine can be handled with try/catch blocks.
     */
    soul::async::Task<std::string> read_text(std::filesystem::path path);

    /**
     * @brief Asynchronously writes UTF-8 text.
     * @param path Destination path.
     * @param text Null-terminated UTF-8 view to persist.
     * @return Awaitable capturing the write result, including any error codes produced by the
     *         underlying backend.
     */
    soul::async::Task<io::WriteFileResult> write_text(std::filesystem::path path,
                                                      std::string_view text);

private:
    std::shared_ptr<io::IAsyncFileIO> m_io;
    std::unique_ptr<Encryption::IEncryptionStrategy> m_encryption;
};

} // namespace soul::filesystem::core
