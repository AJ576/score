# Architecture

Score has a C++ core and two optional integration surfaces:

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
│    private numeric operations                           │
└────────────┬───────────────────────────────────────────┘
             │
             ▼
┌────────────────────────────────────────────────────────┐
│  Optional C ABI                                         │
│    include/score/c_api.h                                │
│    src/c_api.cpp (`extern "C"` wrappers)                │
└────────────────────────────────────────────────────────┘
```

A parallel surface, the **C API** at `include/score/c_api.h`, is built into
the same shared library. It uses opaque pointers and `extern "C"` linkage
so any FFI-capable language (Python via ctypes, Rust, Go, Lua, …) can
drive the same C++ classes.

## Why this shape?

* **One home for math.** Numerical operations live with the C++ domain code.
  This keeps empty-input policy and exception behavior beside the methods
  that define them, without a second internal language boundary.
* **One source of truth for math.** Both nanobind and the C API ultimately
  call the same C++ methods. There is no duplicate implementation of any
  formula.
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
