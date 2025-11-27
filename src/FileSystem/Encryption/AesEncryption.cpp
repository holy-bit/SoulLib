#include "FileSystem/Encryption/AesEncryption.h"
#include <vector>
#include <stdexcept>

namespace FileSystem {
namespace Encryption {

AesEncryption::AesEncryption() {}

void AesEncryption::setKey(const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv) {
    key_ = key;
    iv_ = iv;
}

std::vector<uint8_t> AesEncryption::encrypt(const std::vector<uint8_t>& data) {
    if (key_.empty()) {
        return data;
    }

    std::vector<uint8_t> result = data;
    std::vector<uint8_t> currentKey = key_;
    
    // Simple robust XOR encryption
    // In a real scenario, we would use OpenSSL or a full AES implementation.
    // This is a placeholder implementation that provides obfuscation.
    
    size_t keyIdx = 0;
    size_t ivIdx = 0;
    
    for (size_t i = 0; i < result.size(); ++i) {
        uint8_t k = currentKey[keyIdx];
        uint8_t v = iv_.empty() ? 0 : iv_[ivIdx];
        
        result[i] = result[i] ^ k ^ v;
        
        // Rotate key components to avoid simple repeating patterns
        keyIdx = (keyIdx + 1) % currentKey.size();
        if (!iv_.empty()) {
            ivIdx = (ivIdx + 1) % iv_.size();
        }
        
        // Dynamic key mutation (simple stream cipher behavior)
        if (i % 16 == 0) {
             for(auto& b : currentKey) b = (b << 1) | (b >> 7);
        }
    }
    
    return result;
}

std::vector<uint8_t> AesEncryption::decrypt(const std::vector<uint8_t>& data) {
    // For this symmetric XOR cipher, decryption is identical to encryption
    // IF the key mutation is deterministic and reset.
    // Since encrypt() starts with a fresh copy of key_ and mutates it deterministically,
    // calling encrypt() again on the ciphertext will decrypt it.
    return encrypt(data);
}

} // namespace Encryption
} // namespace FileSystem
