#include <benchmark/benchmark.h>
#include "containers/Sequential/SoulVector.h"
#include "containers/Associative/SoulFlatMap.h"
#include <vector>

static void BM_SoulVectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        ContainerSystem::Sequential::SoulVector<int> data;
        data.reserve(1024);
        for (int i = 0; i < 1024; ++i) {
            data.insert(i);
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_SoulVectorPushBack);

static void BM_StdVectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> data;
        data.reserve(1024);
        for (int i = 0; i < 1024; ++i) {
            data.push_back(i);
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_StdVectorPushBack);

static void BM_SoulFlatMapInsert(benchmark::State& state) {
    ContainerSystem::Associative::SoulFlatMap<int, int> map;
    for (auto _ : state) {
        map.clear();
        for (int i = 0; i < state.range(0); ++i) {
            map.insert({i, i});
        }
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_SoulFlatMapInsert)->Arg(256)->Arg(1024);
