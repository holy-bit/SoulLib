#pragma once
#include <vector>

namespace FileSystem {
namespace Encryption {

class IEncryptionStrategy {
public:
    virtual std::vector<uint8_t> encrypt(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> decrypt(const std::vector<uint8_t>& data) = 0;
    virtual ~IEncryptionStrategy() = default;
};

} // namespace Encryption
} // namespace FileSystem