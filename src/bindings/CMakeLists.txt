# ---------------------------------------------------------------------------
# Python bindings: _score_native (nanobind module)
# ---------------------------------------------------------------------------
#
# nanobind ships its own CMake package config; we locate it by asking the
# active Python interpreter where `pip install nanobind` put it (matches
# whatever scikit-build-core / `make dev` set up in .venv).
# ---------------------------------------------------------------------------

# Locate Python with the components nanobind needs (the Interpreter alone is
# not enough — nanobind_add_module needs Development.Module too).  nanobind's
# CMake config additionally requires a "Python" (not "Python3") package to
# have been found first.
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_VARIABLE _NANOBIND_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _NANOBIND_QUERY_RC
)
if(NOT _NANOBIND_QUERY_RC EQUAL 0)
    message(FATAL_ERROR
        "nanobind not found for ${Python3_EXECUTABLE}.  Run `make dev` (uv + "
        "editable install) so nanobind lands in .venv."
    )
endif()
list(APPEND CMAKE_PREFIX_PATH "${_NANOBIND_CMAKE_DIR}")
find_package(nanobind CONFIG REQUIRED)

nanobind_add_module(_score_native NB_STATIC
    nanobindings.cpp
)

target_link_libraries(_score_native
    PRIVATE
        score_core
)

set_target_properties(_score_native PROPERTIES
    CXX_STANDARD          17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS        OFF
)

# Resolve libscore_core.dylib at runtime relative to the .so itself, so the
# extension works whether it is in build/, in python/score/ next to the
# in-source dylib, or in the wheel install dir alongside it.
if(APPLE)
    set_target_properties(_score_native PROPERTIES
        BUILD_RPATH   "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
elseif(UNIX)
    set_target_properties(_score_native PROPERTIES
        BUILD_RPATH   "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# When building locally (not via scikit-build-core), drop the .so into the
# Python package directory so `python -c "import score"` works without an
# install step.  scikit-build-core will pick it up from the install rules
# below for the wheel.
set_target_properties(_score_native PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/score
)

# Install rules used by scikit-build-core when building wheels
install(TARGETS _score_native
    LIBRARY DESTINATION score
    RUNTIME DESTINATION score
)

# Also install the shared core library next to the module, so the Python
# extension can find it at runtime.
install(TARGETS score_core
    LIBRARY DESTINATION score
    RUNTIME DESTINATION score
)
