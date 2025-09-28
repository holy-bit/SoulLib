#include "FileSystem/Encryption/NoEncryption.h"
#include <vector>

namespace FileSystem {
namespace Encryption {

std::vector<uint8_t> NoEncryption::encrypt(const std::vector<uint8_t>& data) {
    return data;
}

std::vector<uint8_t> NoEncryption::decrypt(const std::vector<uint8_t>& data) {
    return data;
}

} // namespace Encryption
} // namespace FileSystem