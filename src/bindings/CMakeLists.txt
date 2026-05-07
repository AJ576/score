# ---------------------------------------------------------------------------
# Python bindings: _score_native (pybind11 module)
# ---------------------------------------------------------------------------
#
# pybind11 can be located in several ways depending on how Score is being
# built.  We try, in order:
#   1. find_package(pybind11 CONFIG)  - works when pybind11 was installed
#                                       via pip into the active interpreter,
#                                       which scikit-build-core does.
#   2. add_subdirectory(external/pybind11) - if the user vendored it as a
#                                            submodule.
# ---------------------------------------------------------------------------

# Locate Python with the components pybind11 needs (the Interpreter alone is
# not enough — pybind11_add_module uses python3_add_library which requires
# Development.Module).
find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

set(PYBIND11_FINDPYTHON ON)

# First, try the standard CMake config search path.
find_package(pybind11 CONFIG QUIET)

# If that failed, ask the active Python interpreter where the pip-installed
# pybind11 keeps its CMake files and try again.  This makes
#   `pip install pybind11`  →  CMake just works.
if(NOT pybind11_FOUND)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE _PYBIND11_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _PYBIND11_QUERY_RC
    )
    if(_PYBIND11_QUERY_RC EQUAL 0 AND EXISTS "${_PYBIND11_CMAKE_DIR}")
        message(STATUS "Found pip-installed pybind11 at ${_PYBIND11_CMAKE_DIR}")
        find_package(pybind11 CONFIG REQUIRED PATHS "${_PYBIND11_CMAKE_DIR}" NO_DEFAULT_PATH)
    elseif(EXISTS ${CMAKE_SOURCE_DIR}/external/pybind11/CMakeLists.txt)
        add_subdirectory(${CMAKE_SOURCE_DIR}/external/pybind11
                         ${CMAKE_BINARY_DIR}/pybind11)
    else()
        message(FATAL_ERROR
            "pybind11 not found.  Either run inside a venv where "
            "`pip install pybind11` has been run, or vendor it as a submodule "
            "at external/pybind11."
        )
    endif()
endif()

pybind11_add_module(_score_native MODULE
    pybindings.cpp
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
