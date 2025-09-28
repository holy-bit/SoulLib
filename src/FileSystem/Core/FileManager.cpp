#include "FileSystem/Core/FileManager.h"
#include <filesystem>
#include <cstdio>
#include "FileSystem/IO/BinaryFileIO.h"
#include "FileSystem/Encryption/NoEncryption.h"

namespace FileSystem {
namespace Core {

FileManager& FileManager::instance() {
    static FileManager inst;
    return inst;
}

FileManager::FileManager()
    : fileIO(std::make_unique<IO::BinaryFileIO>()),
      encryption(std::make_unique<Encryption::NoEncryption>())
{}

void FileManager::setIOHandler(std::unique_ptr<IO::IFileIO> handler) {
    fileIO = std::move(handler);
}

void FileManager::setEncryptionStrategy(std::unique_ptr<Encryption::IEncryptionStrategy> strategy) {
    encryption = std::move(strategy);
}

bool FileManager::exists(const std::string& path) const {
    return std::filesystem::exists(path);
}

bool FileManager::remove(const std::string& path) {
    return std::filesystem::remove(path);
}

Core::FileHandle FileManager::open(const std::string& path, FileMode mode) {
    if (mode == FileMode::Read)
        return fileIO->openRead(path);
    return fileIO->openWrite(path);
}

void FileManager::close(Core::FileHandle handle) {
    fileIO->close(handle);
}

std::vector<uint8_t> FileManager::readAll(Core::FileHandle handle) {
    auto data = fileIO->read(handle);
    return encryption->decrypt(data);
}

bool FileManager::writeAll(Core::FileHandle handle, const std::vector<uint8_t>& data) {
    auto enc = encryption->encrypt(data);
    return fileIO->write(handle, enc);
}

bool FileManager::save(const std::string& path, const std::vector<uint8_t>& data) {
    auto handle = open(path, FileMode::Write);
    bool ok = writeAll(handle, data);
    close(handle);
    return ok;
}

std::vector<uint8_t> FileManager::load(const std::string& path) {
    auto handle = open(path, FileMode::Read);
    auto data = readAll(handle);
    close(handle);
    return data;
}

bool FileManager::saveJson(const std::string& path, const std::string& json) {
    std::vector<uint8_t> data(json.begin(), json.end());
    return save(path, data);
}

std::string FileManager::loadJson(const std::string& path) {
    auto data = load(path);
    return std::string(data.begin(), data.end());
}

} // namespace Core
} // namespace FileSystem