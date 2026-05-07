"""High-level Python helpers that compose the C++ primitives.

This file is INTENTIONALLY scaffolded with stubs.  The native classes do
the precise numerical work; this layer is where the user assembles them
into useful analyses (e.g. ranking tickers by Sharpe).

Each function below has a complete signature and docstring.  Function
bodies raise :class:`NotImplementedError` until the user fills them in.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

from ._core import DescriptiveStats, Series, StockAnalyzer
from .data import MockData, get_prices


@dataclass(frozen=True)
class TickerSummary:
    """Per-ticker summary produced by :func:`analyze_ticker`."""

    ticker: str
    n_observations: int
    mean_price: float
    price_stddev: float
    cumulative_return: float
    annualized_return: float
    annualized_volatility: float
    sharpe_ratio: float
    max_drawdown: float


def analyze_ticker(
    data: MockData,
    ticker: str,
    *,
    risk_free_rate: float = 0.0,
    periods_per_year: int = 252,
) -> TickerSummary:
    """Compute a full :class:`TickerSummary` for ``ticker``.

    The function calls into the native ``DescriptiveStats`` and
    ``StockAnalyzer`` classes.  Once the C++ stubs are filled in, this
    function returns real numbers; until then, expect the underlying
    methods to raise :class:`score.NativeNotImplementedError`.
    """
    raise NotImplementedError(
        "analyze_ticker: assemble DescriptiveStats(prices) and "
        "StockAnalyzer(prices) and pull the metrics out."
    )


def compare_tickers(
    data: MockData,
    tickers: Iterable[str],
    *,
    risk_free_rate: float = 0.0,
    periods_per_year: int = 252,
) -> list[TickerSummary]:
    """Run :func:`analyze_ticker` over multiple tickers."""
    raise NotImplementedError(
        "compare_tickers: loop over tickers, call analyze_ticker, "
        "and return the list."
    )


def rank_by_sharpe(
    summaries: Iterable[TickerSummary],
    *,
    descending: bool = True,
) -> list[TickerSummary]:
    """Sort summaries by Sharpe ratio."""
    raise NotImplementedError("rank_by_sharpe: sorted(summaries, key=...)")


def correlation_matrix(
    data: MockData,
    tickers: Iterable[str],
    *,
    use_log_returns: bool = True,
) -> list[list[float]]:
    """Pairwise correlation matrix of returns.

    Returns a square 2-D list where ``M[i][j]`` is corr(tickers[i], tickers[j]).
    """
    raise NotImplementedError(
        "correlation_matrix: compute returns per ticker, then "
        "sample Pearson correlation pairwise."
    )
