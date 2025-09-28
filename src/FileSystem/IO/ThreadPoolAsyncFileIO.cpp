#include "FileSystem/IO/ThreadPoolAsyncFileIO.h"

#include <cerrno>
#include <fstream>
#include <system_error>
#include <vector>

namespace soul::filesystem::io {

ThreadPoolAsyncFileIO::ThreadPoolAsyncFileIO(std::shared_ptr<soul::async::TaskScheduler> scheduler)
    : m_scheduler(std::move(scheduler)) {
}

soul::async::Task<ReadFileResult> ThreadPoolAsyncFileIO::read(std::filesystem::path path) {
    auto scheduler = m_scheduler;
    co_return co_await scheduler->run_async([path = std::move(path)]() -> ReadFileResult {
        ReadFileResult result;
        result.path = std::filesystem::absolute(path);

        std::ifstream file(result.path, std::ios::binary);
        if (!file) {
            result.error = std::error_code(errno, std::generic_category());
            return result;
        }

        file.seekg(0, std::ios::end);
        const auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size > 0) {
            result.data.resize(static_cast<std::size_t>(size));
            file.read(reinterpret_cast<char*>(result.data.data()), size);
        }

        if (!file.good() && !file.eof()) {
            result.data.clear();
            result.error = std::error_code(errno, std::generic_category());
        }

        return result;
    });
}

soul::async::Task<WriteFileResult> ThreadPoolAsyncFileIO::write(std::filesystem::path path,
                                                                std::span<const std::byte> data) {
    auto scheduler = m_scheduler;
    std::vector<std::byte> buffer(data.begin(), data.end());
    co_return co_await scheduler->run_async([path = std::move(path), buffer = std::move(buffer)]() mutable {
        WriteFileResult result;
        result.path = std::filesystem::absolute(path);

        std::ofstream file(result.path, std::ios::binary | std::ios::trunc);
        if (!file) {
            result.error = std::error_code(errno, std::generic_category());
            return result;
        }

        if (!buffer.empty()) {
            file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        }

        if (!file.good()) {
            result.error = std::error_code(errno, std::generic_category());
        }

        return result;
    });
}

} // namespace soul::filesystem::io
