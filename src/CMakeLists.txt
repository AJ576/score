# ---------------------------------------------------------------------------
# Core shared library: score_core
# Compiles C++17 sources from core/, statistics/, metrics/, plus the C17
# kernels under c_kernels/ and the extern "C" API in c_api.cpp.
# ---------------------------------------------------------------------------

set(SCORE_CORE_CPP_SOURCES
    core/exceptions.cpp
    core/types.cpp
    statistics/descriptive.cpp
    metrics/stock_metrics.cpp
    c_api.cpp
)

set(SCORE_CORE_C_SOURCES
    c_kernels/kernels.c
)

add_library(score_core SHARED
    ${SCORE_CORE_CPP_SOURCES}
    ${SCORE_CORE_C_SOURCES}
)

target_include_directories(score_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link nlohmann/json (header-only; this propagates the include path)
target_link_libraries(score_core
    PUBLIC
        nlohmann_json::nlohmann_json
)

# Force explicit standards on this target (in case the user overrides globally)
set_target_properties(score_core PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
    C_STANDARD            17
    C_STANDARD_REQUIRED   ON
    C_EXTENSIONS          OFF
    VERSION               ${PROJECT_VERSION}
    SOVERSION             ${PROJECT_VERSION_MAJOR}
)

# Export symbols on Windows (no-op on Unix)
include(GenerateExportHeader)
generate_export_header(score_core
    BASE_NAME SCORE
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/score_export.h
)
target_include_directories(score_core PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
