"""Score: precise stock-market metrics in C/C++ with a Python interface.

The native heavy lifting lives in the compiled extension ``_score_native``
(built from ``src/bindings/pybindings.cpp``).  This package re-exports the
key types so consumers can write::

    from score import Series, DescriptiveStats, StockAnalyzer

without ever touching the underscore-prefixed module directly.
"""

from __future__ import annotations

from ._core import (
    DescriptiveStats,
    DimensionMismatchError,
    DomainError,
    EmptySeriesError,
    NativeNotImplementedError,
    RiskStats,
    Series,
    StockAnalyzer,
    calculate_risk,
    calculate_risk_from_returns,
    log_returns,
)

__all__ = [
    "Series",
    "DescriptiveStats",
    "StockAnalyzer",
    "EmptySeriesError",
    "DimensionMismatchError",
    "DomainError",
    "NativeNotImplementedError",
    "RiskStats",
    "calculate_risk",
    "calculate_risk_from_returns",
    "log_returns",
    "__version__",
]

__version__ = "0.1.0"

