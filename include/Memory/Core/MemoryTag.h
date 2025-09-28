#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Memory::Core {
namespace detail {
    constexpr uint32_t kFNV1aOffset32 = 0x811C9DC5u;
    constexpr uint32_t kFNV1aPrime32 = 0x01000193u;

    constexpr uint32_t hashBytes(const char* data, std::size_t length) noexcept {
        uint32_t hash = kFNV1aOffset32;
        for (std::size_t i = 0; i < length; ++i) {
            hash ^= static_cast<uint8_t>(data[i]);
            hash *= kFNV1aPrime32;
        }
        return hash;
    }
}

struct MemoryTag {
    constexpr MemoryTag() noexcept = default;
    uint32_t hash {0};
#ifdef SOUL_DEBUG
    std::string_view label {};

    constexpr MemoryTag(uint32_t value, std::string_view debugLabel = {}) noexcept
        : hash(value), label(debugLabel) {}
#else
    constexpr MemoryTag(uint32_t value, std::string_view = {}) noexcept
        : hash(value) {}
#endif

    constexpr bool operator==(const MemoryTag& other) const noexcept {
        return hash == other.hash;
    }
    constexpr bool operator!=(const MemoryTag& other) const noexcept {
        return !(*this == other);
    }
};

template <std::size_t N>
constexpr MemoryTag MakeLiteralTag(const char (&literal)[N]) noexcept {
    return MemoryTag(detail::hashBytes(literal, N - 1), std::string_view(literal, N - 1));
}

inline MemoryTag MakeRuntimeTag(std::string_view view) noexcept {
    return MemoryTag(detail::hashBytes(view.data(), view.size()), view);
}

inline MemoryTag MakeRuntimeTag(const char* data, std::size_t length) noexcept {
    return MemoryTag(detail::hashBytes(data, length), std::string_view(data, length));
}

inline uint32_t HashLiteral(std::string_view view) noexcept {
    return detail::hashBytes(view.data(), view.size());
}
}

#define SOUL_MEMORY_TAG(literal) ::Memory::Core::MakeLiteralTag(literal)
#define SOUL_MEMORY_RUNTIME_TAG(text) ::Memory::Core::MakeRuntimeTag(text)
