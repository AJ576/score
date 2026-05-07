# Building

## Prerequisites

* **CMake** ≥ 3.20 (`cmake --version`)
* **A C/C++17 compiler** — Apple Clang 14+, Clang 12+, or GCC 9+
* **Python** ≥ 3.10
* **git** (for the submodule)
* `pybind11`, `numpy`, `pytest` (installed automatically via `pip install -e .[dev]`)

## One-shot build

```bash
git submodule update --init --recursive
python3 -m venv .venv
source .venv/bin/activate
pip install -e ".[dev]"
```

That single `pip install -e` invokes `scikit-build-core`, which in turn
runs CMake, builds `libscore_core` and `_score_native`, and registers the
package in editable mode. After it completes:

```bash
python -c "import score; print(score.__version__)"
```

should print `0.1.0` without errors.

## Manual CMake build

When you want compile errors to land directly in the terminal (instead of
through pip's wrapper), use the Makefile:

```bash
make submodules    # one-time
make build         # configure + build to ./build/
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
make clean         # removes build/
rm -f python/score/_score_native.*  # if a stale .so is left behind
```
