#include <benchmark/benchmark.h>
#include "Memory/Core/PoolAllocator.h"
#include "Memory/Core/ArenaAllocator.h"
#include <cstdint>
#include <vector>

using namespace Memory::Core;

static void BM_PoolAllocatorAllocateDeallocate(benchmark::State& state) {
    PoolAllocator<std::uint32_t, 1024> allocator;
    for (auto _ : state) {
        auto* value = allocator.allocate();
        if (!value) {
            state.SkipWithError("PoolAllocator returned nullptr");
            break;
        }
        *value = 42u;
        benchmark::DoNotOptimize(*value);
        allocator.deallocate(value);
    }
    state.counters["available"] = static_cast<double>(allocator.available());
}
BENCHMARK(BM_PoolAllocatorAllocateDeallocate);

static void BM_PoolAllocatorBatch(benchmark::State& state) {
    PoolAllocator<std::uint32_t, 1024> allocator;
    std::vector<std::uint32_t*> pointers;
    pointers.reserve(512);

    for (auto _ : state) {
        pointers.clear();
        for (int i = 0; i < 512; ++i) {
            auto* ptr = allocator.allocate();
            if (!ptr) {
                state.SkipWithError("PoolAllocator exhausted");
                return;
            }
            *ptr = static_cast<std::uint32_t>(i);
            pointers.push_back(ptr);
        }
        benchmark::DoNotOptimize(pointers);
        for (auto* ptr : pointers) {
            allocator.deallocate(ptr);
        }
    }
}
BENCHMARK(BM_PoolAllocatorBatch);

static void BM_ArenaAllocatorLinear(benchmark::State& state) {
    ArenaAllocator arena(64 * 1024);

    for (auto _ : state) {
        arena.reset();
        for (int i = 0; i < 1024; ++i) {
            auto* ptr = static_cast<std::uint32_t*>(arena.allocate(sizeof(std::uint32_t), alignof(std::uint32_t)));
            *ptr = static_cast<std::uint32_t>(i);
            benchmark::DoNotOptimize(ptr);
        }
    }
}
BENCHMARK(BM_ArenaAllocatorLinear);
