#pragma once
#include "FileSystem/Encryption/IEncryptionStrategy.h"
#include <vector>

namespace FileSystem {
namespace Encryption {

class AesEncryption : public IEncryptionStrategy {
public:
    AesEncryption();
    void setKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);
    std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) override;
private:
    std::vector<uint8_t> key_;
    std::vector<uint8_t> iv_;
};

} // namespace Encryption
} // namespace FileSystem