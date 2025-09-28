# Add test sources
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/time/*.cpp)

# Add test sources for debug
file(GLOB_RECURSE DEBUG_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/debug/*.cpp)

# Include debug test sources
list(APPEND TEST_SOURCES ${DEBUG_TEST_SOURCES})

# Add DebugAPITest explicitly if needed
set(TEST_SOURCES ${TEST_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/debug/DebugAPITest.cpp)

# Collect network test sources
file(GLOB_RECURSE NETWORK_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/network/*.cpp)
list(APPEND TEST_SOURCES ${NETWORK_TEST_SOURCES})

# Collect filesystem tests
file(GLOB_RECURSE FILESYSTEM_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/FileSystem/*.cpp)
list(APPEND TEST_SOURCES ${FILESYSTEM_TEST_SOURCES})

# Collect memory test sources
file(GLOB_RECURSE MEMORY_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Memory/*.cpp)
list(APPEND TEST_SOURCES ${MEMORY_TEST_SOURCES})

# Collect container test sources
file(GLOB_RECURSE CONTAINER_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/containers/*.cpp)
list(APPEND TEST_SOURCES ${CONTAINER_TEST_SOURCES})

# Add the ContainerManager implementation explicitly (source is in src directory, not test)
list(APPEND TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../src/containers/Core/ContainerManager.cpp)

# Create the test executable
add_executable(SoulLibTests ${TEST_SOURCES})

# Link the test executable with the library and Google Test
target_link_libraries(SoulLibTests PRIVATE SoulLib gtest gtest_main)

# Enable testing
include(GoogleTest)
gtest_discover_tests(SoulLibTests)