cmake_minimum_required(VERSION 3.20)

project(SoulLib VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

include(GNUInstallDirs)
include(FetchContent)

option(SOULLIB_BUILD_TESTS "Build SoulLib test suite" ON)
option(SOULLIB_BUILD_BENCHMARKS "Build SoulLib benchmark suite" ON)
option(SOULLIB_BUILD_TOOLS "Build SoulLib developer tools" ON)
option(SOULLIB_BUILD_DOCS "Generate SoulLib API documentation" OFF)
option(SOULLIB_ENABLE_CPP23 "Build SoulLib with the C++23 standard" OFF)
option(SOULLIB_EXPERIMENTAL_MODULES "Build experimental module prototypes" OFF)

set(SOULLIB_CXX_STANDARD 20)
if(SOULLIB_ENABLE_CPP23)
  set(SOULLIB_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_STANDARD ${SOULLIB_CXX_STANDARD})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOULLIB_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")
list(FILTER SOULLIB_SOURCES EXCLUDE REGEX ".*/src/Memory/Modules/MemoryStatisticsImport\\.cpp$")

add_library(SoulLib SHARED ${SOULLIB_SOURCES})
set_target_properties(SoulLib PROPERTIES
  OUTPUT_NAME "SoulLib"
  CXX_STANDARD ${SOULLIB_CXX_STANDARD}
)
add_library(Soul::SoulLib ALIAS SoulLib)

add_library(SoulLibStatic STATIC ${SOULLIB_SOURCES})
set_target_properties(SoulLibStatic PROPERTIES
  OUTPUT_NAME "SoulLib"
  CXX_STANDARD ${SOULLIB_CXX_STANDARD}
)
add_library(Soul::SoulLibStatic ALIAS SoulLibStatic)

target_include_directories(SoulLib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(SoulLibStatic
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(SoulLib PUBLIC cxx_std_20)
target_compile_features(SoulLibStatic PUBLIC cxx_std_20)

if(SOULLIB_BUILD_TESTS)
  FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
      DOWNLOAD_EXTRACT_TIMESTAMP true
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_subdirectory(test)
endif()

if(SOULLIB_BUILD_BENCHMARKS)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
      googlebenchmark
      URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
      DOWNLOAD_EXTRACT_TIMESTAMP true
  )
  FetchContent_MakeAvailable(googlebenchmark)
  add_subdirectory(benchmarks)
endif()

if(SOULLIB_BUILD_TOOLS)
  add_subdirectory(tools/MemoryVisualizer)
endif()

if(SOULLIB_BUILD_DOCS)
  find_package(Doxygen REQUIRED dot)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  add_custom_target(SoulLibDocs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html DESTINATION ${CMAKE_INSTALL_DOCDIR} OPTIONAL)
endif()

if(SOULLIB_EXPERIMENTAL_MODULES)
  if(MSVC)
    set(SOULLIB_MODULE_SOURCES
      src/Memory/Modules/MemoryStatistics.ixx
      src/Memory/Modules/MemoryStatisticsImport.cpp
    )
    set_source_files_properties(src/Memory/Modules/MemoryStatistics.ixx PROPERTIES LANGUAGE CXX)

    add_library(SoulLibMemoryModules STATIC ${SOULLIB_MODULE_SOURCES})
    target_include_directories(SoulLibMemoryModules PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(SoulLibMemoryModules PRIVATE SoulLib)
    target_compile_features(SoulLibMemoryModules PUBLIC cxx_std_20)
    target_compile_options(SoulLibMemoryModules PRIVATE /experimental:module)
    set_target_properties(SoulLibMemoryModules PROPERTIES
      CXX_STANDARD ${SOULLIB_CXX_STANDARD}
      OUTPUT_NAME "SoulLibMemoryModules"
      VS_GLOBAL_EnableModules "true"
      VS_GLOBAL_ScanSourceForModuleDependencies "true"
    )
  else()
    message(WARNING "SOULLIB_EXPERIMENTAL_MODULES is currently supported only when using MSVC.")
  endif()
endif()

install(TARGETS SoulLib SoulLibStatic
  EXPORT SoulLibTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT SoulLibTargets
  FILE SoulLibTargets.cmake
  NAMESPACE Soul::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SoulLib
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/SoulLibConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/SoulLibConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/SoulLibConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SoulLib
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/SoulLibConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/SoulLibConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SoulLib
)


