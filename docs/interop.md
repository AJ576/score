# Interop

Score supports two ways to call into the native code from Python.

## 1. pybind11 (preferred for application code)

```python
from score import Series, DescriptiveStats, StockAnalyzer
from score.data import load_mock_data, get_prices

data = load_mock_data()
prices = get_prices(data, "AAPL")          # Series with 504 doubles
stats  = DescriptiveStats(prices)
sa     = StockAnalyzer(prices)

print(stats.mean(), stats.stddev())
print(sa.cumulative_return(), sa.sharpe_ratio())
```

* **Type-safe** — pybind11 generates the conversion code at C++ compile
  time; passing the wrong type raises `TypeError` at the call site.
* **Idiomatic objects** — `Series` supports `len(s)`, `s[i]`, iteration.
* **No manual destructor calls** — Python's GC handles it.

## 2. C API via `ctypes` (preferred for low-level / FFI practice)

```python
import ctypes
import os

lib = ctypes.CDLL(os.path.join("build", "src", "libscore_core.dylib"))

# ScoreSeries* score_series_create(const double* data, size_t len);
lib.score_series_create.restype  = ctypes.c_void_p
lib.score_series_create.argtypes = [ctypes.POINTER(ctypes.c_double), ctypes.c_size_t]

lib.score_series_destroy.restype  = None
lib.score_series_destroy.argtypes = [ctypes.c_void_p]

lib.score_stats_create.restype  = ctypes.c_void_p
lib.score_stats_create.argtypes = [ctypes.c_void_p]

# ScoreStatus score_stats_mean(const ScoreDescriptiveStats*, double* out);
lib.score_stats_mean.restype  = ctypes.c_int
lib.score_stats_mean.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_double)]

xs = (ctypes.c_double * 5)(1.0, 2.0, 3.0, 4.0, 5.0)
series = lib.score_series_create(xs, len(xs))
stats  = lib.score_stats_create(series)

out = ctypes.c_double()
status = lib.score_stats_mean(stats, ctypes.byref(out))
print(status, out.value)

# … always free in reverse order of creation
lib.score_stats_destroy(stats)
lib.score_series_destroy(series)
```

* **Maximally portable** — works from any language with C FFI.
* **Manual lifecycle** — every `_create` must be matched by `_destroy`.
* **Error codes, not exceptions** — methods return a `ScoreStatus` integer
  and write the result through an out-pointer. Translate via
  `score_status_str` if you want a human-readable message.

## How the same C++ class powers both

Both layers are wrappers over the same C++ instance. Internally:

* **pybind11** holds a `std::unique_ptr<score::statistics::DescriptiveStats>`.
* **The C API** wraps the same class in `struct ScoreDescriptiveStats {
  score::statistics::DescriptiveStats impl; ... };` and hands callers a
  `void*` to it.

When you implement `DescriptiveStats::mean` in
`src/statistics/descriptive.cpp`, both Python entry points immediately
return the new value — there is no second copy of the formula to keep in
sync.

## When to choose which

| You want…                                   | Use         |
|---------------------------------------------|-------------|
| Idiomatic Python in normal app code         | pybind11    |
| Zero-copy NumPy interop                     | pybind11    |
| Practice `ctypes`/`cffi`                    | C API       |
| Drive the library from Rust / Go / Lua      | C API       |
| Stable ABI across Python upgrades           | C API       |
| Pass C-style structs cleanly                | C API       |
