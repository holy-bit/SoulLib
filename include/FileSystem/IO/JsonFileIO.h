#pragma once
#include <string>
#include <vector>
#include "FileSystem/IO/IFileIO.h"

namespace FileSystem {
namespace IO {

class JsonFileIO : public IFileIO {
public:
    JsonFileIO() = default;
    Core::FileHandle openRead(const std::string& path) override;
    Core::FileHandle openWrite(const std::string& path) override;
    std::vector<uint8_t> read(Core::FileHandle handle) override;
    bool write(Core::FileHandle handle, const std::vector<uint8_t>& data) override;
    void close(Core::FileHandle handle) override;
};

} // namespace IO
} // namespace FileSystem