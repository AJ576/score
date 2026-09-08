# Score

Precise stock-market statistics and metrics in C++17, available to Python
through nanobind. Score is a learning project for numerical methods,
mathematical statistics, object-oriented design, and language interop.

## Architecture

```
Python package
      |
      v
nanobind module (_score_native)
      |
      v
C++17 core: Series, statistics, metrics
      ^
      |
optional C ABI: c_api.h / c_api.cpp
```

The C++ core is the single home for math. `DescriptiveStats` uses standard
C++ algorithms and local numeric operations; there is no separate C kernel
layer. The optional C ABI is implemented in C++ with `extern "C"` linkage,
opaque handles, and exception-to-status translation for consumers such as
ctypes, Rust, or Go.

## Layout

```
score/
├── include/score/        # C++ public headers and optional c_api.h
├── src/
│   ├── core/             # Series<T>, exceptions
│   ├── statistics/       # DescriptiveStats and distributions
│   ├── metrics/          # StockAnalyzer
│   ├── c_api.cpp         # C ABI implemented in C++
│   └── bindings/         # nanobind module
├── python/score/         # Python package
├── tests/                # Catch2 and pytest suites
├── docs/                 # architecture, build, and interop notes
└── external/json/        # nlohmann/json submodule
```

## Quick Start

```bash
git submodule update --init --recursive
make build
.venv/bin/python -c "import score; print(score.__version__)"
make test-cpp
make test-py
```

Regenerate mock market data with:

```bash
python scripts/gen_mock_data.py
```

## Nanobind Direction

Python uses the native C++ domain types directly through `_score_native`:

1. Keep `Series<double>`, `DescriptiveStats`, and `StockAnalyzer` as the
   canonical implementation.
2. Add or evolve Python-facing behavior in
   `src/bindings/nanobindings.cpp`, binding C++ methods rather than the C ABI.
3. Cover native behavior with Catch2 and Python ownership/conversion behavior
   with pytest.

`c_api.h` remains supported as a separate ABI for non-Python FFI clients; it
is not an internal dependency of nanobind.

See `docs/architecture.md`, `docs/building.md`, and `docs/interop.md` for
details.

## License

MIT
