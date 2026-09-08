cmake_minimum_required(VERSION 3.20)

project(score
    VERSION 0.1.0
    DESCRIPTION "Score: precise stock-market metrics in C/C++ with a Python interface"
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------------
# Standards: C17 and C++17, both required, no compiler extensions.
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Helpful for IDE / clangd integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(SCORE_BUILD_NANOBIND "Build the nanobind Python module" ON)
option(SCORE_BUILD_TESTS  "Build the C++ tests"               ON)

# Position-independent code, required for shared libs / Python modules
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable link-time optimization where supported so hot leaf functions defined
# in one translation unit (e.g. score::statistics) can still be inlined into
# callers in another (e.g. the nanobind module).
include(CheckIPOSupported)
check_ipo_supported(RESULT SCORE_IPO_SUPPORTED OUTPUT SCORE_IPO_ERROR)
if(SCORE_IPO_SUPPORTED AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

# ---------------------------------------------------------------------------
# Warnings (apply at directory scope to all our targets, not to vendored deps)
# ---------------------------------------------------------------------------
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# nlohmann/json is added as a git submodule under external/json.
# It supplies the imported target nlohmann_json::nlohmann_json.
if(EXISTS ${CMAKE_SOURCE_DIR}/external/json/CMakeLists.txt)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    add_subdirectory(external/json)
else()
    message(FATAL_ERROR
        "external/json/CMakeLists.txt not found. "
        "Run: git submodule update --init --recursive"
    )
endif()

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
add_subdirectory(src)

if(SCORE_BUILD_NANOBIND)
    add_subdirectory(src/bindings)
endif()

if(SCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/cpp)
endif()
