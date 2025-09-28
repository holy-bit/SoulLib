#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>

namespace FileSystem {
namespace Core {

using FileHandle = std::FILE*;
enum class FileMode { Read, Write, ReadWrite };

class IFileManager {
public:
    virtual bool exists(const std::string& path) const = 0;
    virtual bool remove(const std::string& path) = 0;
    virtual FileHandle open(const std::string& path, FileMode mode) = 0;
    virtual void close(FileHandle handle) = 0;
    virtual std::vector<uint8_t> readAll(FileHandle handle) = 0;
    virtual bool writeAll(FileHandle handle, const std::vector<uint8_t>& data) = 0;
    virtual ~IFileManager() = default;
};

} // namespace Core
} // namespace FileSystem