#include "FileSystem/Core/AsyncFileManager.h"

#include <cstring>
#include <string_view>
#include <system_error>
#include <vector>

namespace soul::filesystem::core {

namespace Encryption = ::FileSystem::Encryption;

AsyncFileManager::AsyncFileManager(std::shared_ptr<io::IAsyncFileIO> ioBackend,
                                   std::unique_ptr<Encryption::IEncryptionStrategy> encryption)
    : m_io(std::move(ioBackend)),
      m_encryption(std::move(encryption)) {
    if (!m_encryption) {
        m_encryption = std::make_unique<Encryption::NoEncryption>();
    }
}

soul::async::Task<io::ReadFileResult> AsyncFileManager::read(std::filesystem::path path) {
    auto result = co_await m_io->read(std::move(path));
    if (result.error) {
        co_return result;
    }

    auto decrypted = m_encryption->decrypt(std::vector<uint8_t>(
        reinterpret_cast<const uint8_t*>(result.data.data()),
        reinterpret_cast<const uint8_t*>(result.data.data()) + result.data.size()));

    result.data.assign(reinterpret_cast<const std::byte*>(decrypted.data()),
                       reinterpret_cast<const std::byte*>(decrypted.data() + decrypted.size()));
    co_return result;
}

soul::async::Task<io::WriteFileResult> AsyncFileManager::write(std::filesystem::path path,
                                                               std::span<const std::byte> data) {
    std::vector<uint8_t> plain(data.size());
    if (!data.empty()) {
        std::memcpy(plain.data(), data.data(), data.size());
    }
    auto encrypted = m_encryption->encrypt(plain);
    std::vector<std::byte> buffer(encrypted.size());
    std::memcpy(buffer.data(), encrypted.data(), encrypted.size());
    auto writeResult = co_await m_io->write(std::move(path), buffer);
    co_return writeResult;
}

soul::async::Task<std::string> AsyncFileManager::read_text(std::filesystem::path path) {
    auto result = co_await read(std::move(path));
    if (result.error) {
        throw std::system_error(result.error);
    }
    co_return std::string(reinterpret_cast<const char*>(result.data.data()), result.data.size());
}

soul::async::Task<io::WriteFileResult> AsyncFileManager::write_text(std::filesystem::path path,
                                                                    std::string_view text) {
    std::span<const std::byte> bytes(reinterpret_cast<const std::byte*>(text.data()), text.size());
    auto writeResult = co_await write(std::move(path), bytes);
    co_return writeResult;
}

} // namespace soul::filesystem::core
