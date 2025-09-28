#pragma once

#include <filesystem>
#include <memory>

#include "Async/Task.h"
#include "FileSystem/IO/IAsyncFileIO.h"

namespace soul::filesystem::io {

/**
 * @brief Default implementation of @ref IAsyncFileIO that performs blocking file work on a task scheduler.
 */
class ThreadPoolAsyncFileIO final : public IAsyncFileIO {
public:
    explicit ThreadPoolAsyncFileIO(std::shared_ptr<soul::async::TaskScheduler> scheduler);

    soul::async::Task<ReadFileResult> read(std::filesystem::path path) override;
    soul::async::Task<WriteFileResult> write(std::filesystem::path path,
                                             std::span<const std::byte> data) override;

private:
    std::shared_ptr<soul::async::TaskScheduler> m_scheduler;
};

} // namespace soul::filesystem::io
