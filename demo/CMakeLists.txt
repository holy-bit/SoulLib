cmake_minimum_required(VERSION 3.15)
project(DemoProject)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add include directories, including subdirectories like 'time'
include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/../include/time)

# Find the SoulLib library
find_library(SOUL_LIB SoulLib PATHS ${CMAKE_SOURCE_DIR}/../build/bin/Debug/)

# Add the main.cpp file
set(SOURCES main.cpp)

# Create the executable
add_executable(main ${SOURCES})

# Link the SoulLib library
if(SOUL_LIB)
    target_link_libraries(main ${SOUL_LIB})
else()
    message(FATAL_ERROR "SoulLib library not found!")
endif()