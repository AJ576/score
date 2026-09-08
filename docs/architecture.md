# Architecture

Score is structured as four layers, each with a single responsibility:

```
┌────────────────────────────────────────────────────────┐
│  Python user-facing package    (python/score/)         │
│    • analysis.py — high-level helpers (stubs)          │
│    • data.py     — JSON mock-data loader               │
│    • _core.py    — re-exports from native module       │
└────────────┬───────────────────────────────────────────┘
             │
             ▼
┌────────────────────────────────────────────────────────┐
│  nanobind module: _score_native                         │
│    src/bindings/nanobindings.cpp                        │
│    Translates std::vector ↔ list/numpy automatically.  │
└────────────┬───────────────────────────────────────────┘
             │
             ▼
┌────────────────────────────────────────────────────────┐
│  C++17 core library: libscore_core                     │
│    score::Series<T>                                    │
│    score::statistics::DescriptiveStats                 │
│    score::metrics::StockAnalyzer                       │
│    score::*Exception hierarchy                         │
└────────────┬───────────────────────────────────────────┘
             │
             ▼
┌────────────────────────────────────────────────────────┐
│  C17 numerical primitives                              │
│    score_k_sum, score_k_dot,                           │
│    score_k_min, score_k_max,                           │
│    score_k_accumulate_squared_dev                      │
└────────────────────────────────────────────────────────┘
```

A parallel surface, the **C API** at `include/score/c_api.h`, is built into
the same shared library. It uses opaque pointers and `extern "C"` linkage
so any FFI-capable language (Python via ctypes, Rust, Go, Lua, …) can
drive the same C++ classes.

## Why this shape?

* **Separation of math and plumbing.** The C++ classes hold no algorithmic
  logic of their own — they delegate to the C kernels for sums, dot
  products, etc. That makes the C/C++ split visible and useful, instead of
  making C a thin afterthought.
* **One source of truth for math.** Both nanobind and the C API ultimately
  call the same C++ methods, which call the same C kernels. There is no
  duplicate implementation of any formula.
* **Stable ABI option.** nanobind ties the binary to a specific Python
  ABI (with an opt-in stable-ABI mode for CPython 3.12+). The C API doesn't,
  so the same `libscore_core.so` can be loaded
  from any language with a C FFI without recompiling.
* **Templates where useful, not pervasive.** `Series<T>` is a template, but
  the public surface only specializes it to `double` (`SeriesD`). Avoids
  template bloat in the binary while keeping the option open.

## Object lifetime

The two analysis classes (`DescriptiveStats`, `StockAnalyzer`) hold a
**const reference** to their input `Series`. Callers must keep the series
alive as long as the analyzer. Both nanobind (via `nb::keep_alive<1, 2>`)
and the C API (by storing a non-owning pointer next to the analyzer
struct) handle this transparently for their respective callers.

## Build artifacts

After `make build`:

```
build/
├── src/
│   └── libscore_core.{so,dylib}        # the C/C++ shared library
├── src/bindings/
│   └── _score_native.cpython-*.{so}    # the nanobind extension
└── tests/cpp/
    └── score_tests                     # the Catch2 binary
```

`scikit-build-core` repackages those into a wheel under `dist/` when you
run `pip install -e .` or `python -m build`.
