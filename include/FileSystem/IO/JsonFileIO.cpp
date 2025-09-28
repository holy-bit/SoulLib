#include "FileSystem/IO/JsonFileIO.h"
#include <cstdio>
#include <vector>
#include <cstdint>

namespace FileSystem {
namespace IO {

Core::FileHandle JsonFileIO::openRead(const std::string& path) {
    return std::fopen(path.c_str(), "r");
}

Core::FileHandle JsonFileIO::openWrite(const std::string& path) {
    return std::fopen(path.c_str(), "w");
}

std::vector<uint8_t> JsonFileIO::read(Core::FileHandle handle) {
    std::vector<uint8_t> buffer;
    if (!handle) return buffer;
    std::fseek(handle, 0, SEEK_END);
    long size = std::ftell(handle);
    std::fseek(handle, 0, SEEK_SET);
    if (size > 0) {
        buffer.resize(size);
        std::fread(buffer.data(), 1, size, handle);
    }
    return buffer;
}

bool JsonFileIO::write(Core::FileHandle handle, const std::vector<uint8_t>& data) {
    if (!handle) return false;
    size_t written = std::fwrite(data.data(), 1, data.size(), handle);
    return written == data.size();
}

void JsonFileIO::close(Core::FileHandle handle) {
    if (handle) std::fclose(handle);
}

} // namespace IO
} // namespace FileSystem