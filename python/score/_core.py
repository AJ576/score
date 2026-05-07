"""Thin wrapper that imports symbols from the compiled extension.

Keeping this in a separate module makes it easy to:
  * stub the imports out in tests if the extension is not built,
  * add Pythonic conveniences (``__repr__``, helper classmethods, etc.)
    around the native classes without polluting the binding C++ code.
"""

from __future__ import annotations

try:
    from score._score_native import (  # type: ignore[import-not-found]
        DescriptiveStats,
        DimensionMismatchError,
        DomainError,
        EmptySeriesError,
        NativeNotImplementedError,
        Series,
        StockAnalyzer,
    )
except ImportError as exc:  # pragma: no cover - defensive for fresh checkouts
    raise ImportError(
        "Could not import the compiled '_score_native' extension. "
        "Build it first with `make build` or `pip install -e .`."
    ) from exc


__all__ = [
    "DescriptiveStats",
    "DimensionMismatchError",
    "DomainError",
    "EmptySeriesError",
    "NativeNotImplementedError",
    "Series",
    "StockAnalyzer",
]
