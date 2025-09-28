#pragma once
#include "FileSystem/Encryption/IEncryptionStrategy.h"

namespace FileSystem {
namespace Encryption {

class NoEncryption : public IEncryptionStrategy {
public:
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;
};

} // namespace Encryption
} // namespace FileSystem