set(BENCHMARK_SOURCES
    BenchmarkMain.cpp
    MemoryBenchmarks.cpp
    ContainersBenchmarks.cpp
)

add_executable(SoulLibBenchmarks ${BENCHMARK_SOURCES})

target_link_libraries(SoulLibBenchmarks
    PRIVATE
        SoulLibStatic
        benchmark::benchmark
)

target_compile_features(SoulLibBenchmarks PRIVATE cxx_std_17)
