#pragma once
#include <string>
#include <vector>
#include "FileSystem/Core/IFileManager.h"

namespace FileSystem {
namespace IO {

class IFileIO {
public:
    virtual Core::FileHandle openRead(const std::string& path) = 0;
    virtual Core::FileHandle openWrite(const std::string& path) = 0;
    virtual std::vector<uint8_t> read(Core::FileHandle handle) = 0;
    virtual bool write(Core::FileHandle handle, const std::vector<uint8_t>& data) = 0;
    virtual void close(Core::FileHandle handle) = 0;
    virtual ~IFileIO() = default;
};

} // namespace IO
} // namespace FileSystem