#include <gtest/gtest.h>
#include "FileSystem/IO/BinaryFileIO.h"
#include <cstdio>
#include <vector>
#include <filesystem>

using namespace FileSystem::IO;

TEST(BinaryFileIOTest, WriteAndReadBinaryFile) {
    BinaryFileIO fileIO;
    std::string path = "test_binary_file.dat";
    std::vector<uint8_t> data = {10, 20, 30, 40, 50};

    // Write data to file
    auto handle = fileIO.openWrite(path);
    ASSERT_NE(handle, nullptr);
    ASSERT_TRUE(fileIO.write(handle, data));
    fileIO.close(handle);

    // Read data back from file
    handle = fileIO.openRead(path);
    ASSERT_NE(handle, nullptr);
    auto readData = fileIO.read(handle);
    fileIO.close(handle);

    EXPECT_EQ(data, readData);

    // Clean up
    std::filesystem::remove(path);
}

TEST(BinaryFileIOTest, HandleInvalidFileOperations) {
    BinaryFileIO fileIO;

    // Attempt to read from an invalid handle
    auto readData = fileIO.read(nullptr);
    EXPECT_TRUE(readData.empty());

    // Attempt to write to an invalid handle
    std::vector<uint8_t> data = {1, 2, 3};
    EXPECT_FALSE(fileIO.write(nullptr, data));
}