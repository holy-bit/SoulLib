#include <gtest/gtest.h>
#include "FileSystem/Encryption/NoEncryption.h"

using namespace FileSystem::Encryption;

TEST(NoEncryptionTest, EncryptReturnsInputUnchanged) {
    NoEncryption noEnc;
    std::vector<uint8_t> input = {1, 2, 3, 4, 5};
    auto output = noEnc.encrypt(input);
    EXPECT_EQ(input, output);
}

TEST(NoEncryptionTest, DecryptReturnsInputUnchanged) {
    NoEncryption noEnc;
    std::vector<uint8_t> input = {10, 20, 30, 40, 50};
    auto output = noEnc.decrypt(input);
    EXPECT_EQ(input, output);
}