cmake_minimum_required(VERSION 3.20)

add_executable(SoulLibDagViz DagVisualizer.cpp)

target_link_libraries(SoulLibDagViz PRIVATE SoulLib)

target_compile_features(SoulLibDagViz PRIVATE cxx_std_20)

set_target_properties(SoulLibDagViz PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)
