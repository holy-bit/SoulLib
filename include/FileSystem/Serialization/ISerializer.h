#pragma once
#include <vector>

namespace FileSystem {
namespace Serialization {

/**
 * @brief Interface that defines serialization contracts for value types.
 *
 * @tparam T Value type handled by the serializer implementation.
 */
template<typename T>
class ISerializer {
public:
    /**
     * @brief Serializes an object into a contiguous buffer.
     * @param obj Instance to encode.
     * @return Raw bytes representing @p obj.
     */
    virtual std::vector<uint8_t> serialize(const T& obj) = 0;

    /**
     * @brief Restores an object from a contiguous buffer.
     * @param data Raw data previously produced by serialize().
     * @return Deserialized value of type @tparam T.
     */
    virtual T deserialize(const std::vector<uint8_t>& data) = 0;

    /// Virtual destructor for correct polymorphic cleanup.
    virtual ~ISerializer() = default;
};

} // namespace Serialization
} // namespace FileSystem