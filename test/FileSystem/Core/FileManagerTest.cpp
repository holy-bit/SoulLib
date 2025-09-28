#include <gtest/gtest.h>
#include <filesystem>
#include "FileSystem/Core/FileManager.h"

using namespace FileSystem::Core;

TEST(FileManagerTest, SaveLoadBinary) {
    auto& fm = FileManager::instance();
    std::string path = "test_bin.dat";
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};

    ASSERT_TRUE(fm.save(path, data));
    EXPECT_TRUE(fm.exists(path));

    auto loaded = fm.load(path);
    EXPECT_EQ(data, loaded);

    ASSERT_TRUE(fm.remove(path));
    EXPECT_FALSE(fm.exists(path));
}

TEST(FileManagerTest, SaveLoadJson) {
    auto& fm = FileManager::instance();
    std::string path = "test_json.txt";
    std::string json = "{\"key\":\"value\"}";

    ASSERT_TRUE(fm.saveJson(path, json));
    EXPECT_TRUE(fm.exists(path));

    auto loadedJson = fm.loadJson(path);
    EXPECT_EQ(json, loadedJson);

    ASSERT_TRUE(fm.remove(path));
    EXPECT_FALSE(fm.exists(path));
}