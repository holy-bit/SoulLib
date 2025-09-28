#pragma once
#include <memory>
#include <string>
#include <vector>
#include "FileSystem/Core/IFileManager.h"
#include "FileSystem/IO/IFileIO.h"
#include "FileSystem/Encryption/IEncryptionStrategy.h"

namespace FileSystem {
namespace Core {

class FileManager : public IFileManager {
public:
    static FileManager& instance();

    void setIOHandler(std::unique_ptr<IO::IFileIO> handler);
    void setEncryptionStrategy(std::unique_ptr<Encryption::IEncryptionStrategy> strategy);

    bool exists(const std::string& path) const override;
    bool remove(const std::string& path) override;
    FileHandle open(const std::string& path, FileMode mode) override;
    void close(FileHandle handle) override;
    std::vector<uint8_t> readAll(FileHandle handle) override;
    bool writeAll(FileHandle handle, const std::vector<uint8_t>& data) override;

    bool save(const std::string& path, const std::vector<uint8_t>& data);
    std::vector<uint8_t> load(const std::string& path);

    // Handle JSON as raw text without external library
    bool saveJson(const std::string& path, const std::string& json);
    std::string loadJson(const std::string& path);

private:
    FileManager();
    std::unique_ptr<IO::IFileIO> fileIO;
    std::unique_ptr<Encryption::IEncryptionStrategy> encryption;
};

} // namespace Core
} // namespace FileSystem