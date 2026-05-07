"""Verifies the pybind11 module loads and the type wiring is correct.

These tests deliberately avoid asserting numerical results (the C++
methods are stubs).  They check structure, not math.
"""

from __future__ import annotations

import pytest

from score import (
    DescriptiveStats,
    Series,
    StockAnalyzer,
    __version__,
)


def test_version_string() -> None:
    assert __version__ == "0.1.0"


def test_series_construct_empty() -> None:
    s = Series()
    assert len(s) == 0
    assert s.empty() is True


def test_series_construct_from_list(small_floats: list[float]) -> None:
    s = Series(small_floats)
    assert len(s) == len(small_floats)
    assert list(s) == small_floats


def test_series_construct_with_dates(small_floats: list[float]) -> None:
    dates = ["2024-01-01", "2024-01-02", "2024-01-03", "2024-01-04", "2024-01-05"]
    s = Series(small_floats, dates)
    assert len(s) == 5
    assert s.dates() == dates


def test_series_indexing(small_floats: list[float]) -> None:
    s = Series(small_floats)
    assert s[0] == 1.0
    assert s[4] == 5.0
    with pytest.raises(IndexError):
        _ = s[42]


def test_descriptive_stats_constructs(small_floats: list[float]) -> None:
    s = Series(small_floats)
    d = DescriptiveStats(s)
    # We don't assert on numerical results until the C++ math is filled in.
    # We only check that the call dispatches without crashing.
    _ = d


def test_stock_analyzer_constructs(known_prices: list[float]) -> None:
    s = Series(known_prices)
    sa = StockAnalyzer(s)
    _ = sa
