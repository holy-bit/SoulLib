#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "Async/Task.h"
#include "FileSystem/Core/AsyncFileManager.h"
#include "FileSystem/IO/ThreadPoolAsyncFileIO.h"

namespace fs = std::filesystem;

TEST(AsyncFileManager, ReadWriteRoundTrip) {
    auto scheduler = std::make_shared<soul::async::TaskScheduler>(2);
    auto ioBackend = std::make_shared<soul::filesystem::io::ThreadPoolAsyncFileIO>(scheduler);
    soul::filesystem::core::AsyncFileManager manager(ioBackend);

    const fs::path tempPath = fs::temp_directory_path() / "soul_async_fs_roundtrip.bin";
    std::vector<std::byte> payload = {std::byte{0x1}, std::byte{0x2}, std::byte{0x3}, std::byte{0x4}};

    auto writeResult = manager.write(tempPath, payload).get();
    ASSERT_FALSE(writeResult.error) << writeResult.error.message();

    auto readResult = manager.read(tempPath).get();
    ASSERT_FALSE(readResult.error) << readResult.error.message();
    EXPECT_EQ(readResult.data, payload);

    fs::remove(tempPath);
}

TEST(AsyncFileManager, ReadTextMatchesWrite) {
    auto scheduler = std::make_shared<soul::async::TaskScheduler>(1);
    auto ioBackend = std::make_shared<soul::filesystem::io::ThreadPoolAsyncFileIO>(scheduler);
    soul::filesystem::core::AsyncFileManager manager(ioBackend);

    const fs::path tempPath = fs::temp_directory_path() / "soul_async_fs_text.txt";
    std::string text = "SoulLib async IO works";

    auto writeResult = manager.write_text(tempPath, text).get();
    ASSERT_FALSE(writeResult.error);

    auto readText = manager.read_text(tempPath).get();
    EXPECT_EQ(readText, text);

    fs::remove(tempPath);
}
