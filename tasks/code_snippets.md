# C++ Kernel Migration Notes

`DescriptiveStats::variance()` now keeps its central-moment computation next
to its empty-series policy, using standard C++ algorithms:

```cpp
const double sum_sq = std::accumulate(series_.cbegin(), series_.cend(), 0.0,
                                      [m](double sum, double value) {
                                          const double d = value - m;
                                          return sum + d * d;
                                      });
```

This replaces the deleted private C kernel and avoids a second language layer
without changing the C ABI.