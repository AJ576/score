# Building

## Prerequisites

* **CMake** ≥ 3.20 (`cmake --version`)
* **A C/C++17 compiler** — Apple Clang 14+, Clang 12+, or GCC 9+
* **Python** ≥ 3.10
* **git** (for the submodule)
* **[uv](https://docs.astral.sh/uv/)** (creates `.venv` and installs Python deps)
* `pybind11`, `numpy`, `pytest` (installed automatically via `make dev`)

## One-shot build

```bash
git submodule update --init --recursive
make build    # clean, uv dev env, configure, and compile
```

`make build` creates `.venv` via uv, installs the editable package (which
pulls in `pybind11`, `numpy`, `pytest`, etc.), removes stale build
artifacts, then runs CMake. After it completes:

```bash
.venv/bin/python -c "import score; print(score.__version__)"
```

should print `0.1.0` without errors.

## Manual CMake build

When you want compile errors to land directly in the terminal (instead of
through pip's wrapper), use the Makefile:

```bash
make build         # clean + dev + configure + build
make test-cpp      # ctest from build dir
make test-py       # pytest tests/python
```

Or invoke CMake directly:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Build outputs

```
build/
├── src/
│   └── libscore_core.{so,dylib}
├── src/bindings/
│   └── _score_native.cpython-XYZ.{so}
└── tests/cpp/
    └── score_tests
```

The Python extension is also copied into `python/score/` so that
`python -c "import score"` works even without `pip install -e .` (handy
when you want to iterate on a single C++ file without re-running pip).

## Useful options

| Flag                              | Default | Effect                                  |
|-----------------------------------|---------|-----------------------------------------|
| `-DCMAKE_BUILD_TYPE=Debug`        | Release | Disables optimization, adds debug info  |
| `-DSCORE_BUILD_TESTS=OFF`         | ON      | Skip Catch2 fetch + test executable     |
| `-DSCORE_BUILD_PYBIND=OFF`        | ON      | Skip the pybind11 module (C/C++ only)   |

## Cleaning

```bash
make clean         # removes build/ and stale python/score/_score_native.*
```
