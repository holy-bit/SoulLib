#pragma once
#include "FileSystem/Serialization/ISerializer.h"
#include <vector>
#include <cstring>
#include <algorithm>

namespace FileSystem {
namespace Serialization {

template<typename T>
class BinarySerializer : public ISerializer<T> {
public:
    /**
     * @brief Encodes @p obj as a raw binary blob using a shallow memory copy.
     * @note The function assumes @tparam T is trivially copyable.
     */
    std::vector<uint8_t> serialize(const T& obj) override {
        std::vector<uint8_t> buffer(sizeof(T));
        std::memcpy(buffer.data(), &obj, sizeof(T));
        return buffer;
    }

    /**
     * @brief Decodes a binary blob into a value of type @tparam T.
     * @param data Buffer previously produced by serialize().
     * @return A reconstructed value with bytes copied from @p data.
     */
    T deserialize(const std::vector<uint8_t>& data) override {
        T obj{};
        std::memcpy(&obj, data.data(), std::min(data.size(), sizeof(T)));
        return obj;
    }
};

} // namespace Serialization
} // namespace FileSystem