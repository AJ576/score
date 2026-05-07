# ---------------------------------------------------------------------------
# C++ tests via Catch2 v3 (FetchContent — no system install needed)
# ---------------------------------------------------------------------------

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
)
FetchContent_MakeAvailable(Catch2)

# Catch2 ships a CMake module to auto-discover TEST_CASEs
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(score_tests
    test_kernels.cpp
    test_statistics.cpp
    test_metrics.cpp
)

target_link_libraries(score_tests
    PRIVATE
        score_core
        Catch2::Catch2WithMain
)

set_target_properties(score_tests PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
)

catch_discover_tests(score_tests)
