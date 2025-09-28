add_executable(SoulLibMemoryViz main.cpp)

target_link_libraries(SoulLibMemoryViz PRIVATE Soul::SoulLib)

set_target_properties(SoulLibMemoryViz PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
