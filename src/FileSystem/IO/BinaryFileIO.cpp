#include "FileSystem/IO/BinaryFileIO.h"
#include <cstdio>
#include <vector>

namespace FileSystem {
namespace IO {

Core::FileHandle BinaryFileIO::openRead(const std::string& path) {
    return std::fopen(path.c_str(), "rb");
}

Core::FileHandle BinaryFileIO::openWrite(const std::string& path) {
    return std::fopen(path.c_str(), "wb");
}

std::vector<uint8_t> BinaryFileIO::read(Core::FileHandle handle) {
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

bool BinaryFileIO::write(Core::FileHandle handle, const std::vector<uint8_t>& data) {
    if (!handle) return false;
    size_t written = std::fwrite(data.data(), 1, data.size(), handle);
    return written == data.size();
}

void BinaryFileIO::close(Core::FileHandle handle) {
    if (handle) std::fclose(handle);
}

} // namespace IO
} // namespace FileSystem