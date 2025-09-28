#pragma once

#include "Memory/Core/MemoryTag.h"

namespace ContainerSystem::Core {
    struct VectorTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulVector"); }
        static constexpr const char* name() { return "SoulVector"; }
    };

    struct DequeTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulDeque"); }
        static constexpr const char* name() { return "SoulDeque"; }
    };

    struct ListTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulList"); }
        static constexpr const char* name() { return "SoulList"; }
    };

    struct ForwardListTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulForwardList"); }
        static constexpr const char* name() { return "SoulForwardList"; }
    };

    struct UnorderedMapTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulUnorderedMap"); }
        static constexpr const char* name() { return "SoulUnorderedMap"; }
    };

    struct UnorderedSetTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulUnorderedSet"); }
        static constexpr const char* name() { return "SoulUnorderedSet"; }
    };

    struct FlatMapTag {
        static constexpr Memory::Core::MemoryTag value() { return SOUL_MEMORY_TAG("SoulFlatMap"); }
        static constexpr const char* name() { return "SoulFlatMap"; }
    };
}
