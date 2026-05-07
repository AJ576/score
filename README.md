# Score

Precise stock-market metrics in C and C++17, exposed to Python via pybind11
and a flat C API. Intended as a long-running learning project covering
numerical methods, mathematical statistics, OOP/OOD, and language interop.

## Layout

```
score/
├── include/score/        # public C / C++ headers
├── src/                  # C++ implementation, C kernels, pybind11, C API
│   ├── core/             # Series<T>, exceptions
│   ├── statistics/       # DescriptiveStats
│   ├── metrics/          # StockAnalyzer
│   ├── c_kernels/        # pure C17 numerical primitives
│   ├── c_api.cpp         # extern "C" wrappers
│   └── bindings/         # pybind11 module
├── python/score/         # Python package
├── tests/cpp             # Catch2 tests
├── tests/python          # pytest tests
├── data/                 # mock stock JSON
├── scripts/              # mock-data generator, build helper
├── docs/                 # architecture / interop / build docs
└── external/json/        # nlohmann/json submodule
```

## Quick start

```bash
# 1. Pull the JSON submodule
git submodule update --init --recursive

# 2. Set up a venv with Python deps
python3 -m venv .venv
source .venv/bin/activate
pip install pybind11 numpy pytest

# 3. Build the native library + Python extension and install in editable mode
pip install -e ".[dev]"

# 4. Smoke-test the import
python -c "import score; print(score.__version__)"

# 5. Run tests
make test-cpp     # Catch2; most tests are [!shouldfail] until math is implemented
make test-py      # pytest; data + binding tests pass immediately

# 6. (Re)generate mock data
python scripts/gen_mock_data.py
```

## Architecture

Three layers, single source of truth:

```
Python
   ↓  pybind11
C++17 (score::Series, statistics, metrics)
   ↓
C17 kernels (sum, dot, accumulate)
```

A separate `extern "C"` API at `include/score/c_api.h` exposes the same
C++ classes through opaque pointers, so `ctypes` / `cffi` / Rust / Go can
also drive the library.

See `docs/architecture.md` and `docs/interop.md` for details.

## Implementation status

The scaffold is intentionally *complete in shape* and *empty in math*:

| Layer                 | State                              |
|-----------------------|------------------------------------|
| Build system          | complete (CMake + Make + scikit-build-core) |
| C++ headers           | complete (full declarations + Doxygen)      |
| C++ method bodies     | **stubs** (return 0.0 / throw NotImpl)      |
| C kernel bodies       | **stubs**                                   |
| C API forwarding      | complete                                    |
| pybind11 bindings     | complete                                    |
| Python package        | complete                                    |
| Mock data + loader    | complete                                    |
| C++ tests             | written, mostly tagged `[!shouldfail]`      |
| Python tests          | data + bindings pass immediately            |

Recommended order to fill in the math:

1. `src/c_kernels/kernels.c` — the rest of the library leans on these
2. `src/statistics/descriptive.cpp` — uses the kernels
3. `src/metrics/stock_metrics.cpp` — uses descriptive stats
4. As each function is implemented, drop the `[!shouldfail]` tag from the
   matching test in `tests/cpp/`.

## License

MIT
