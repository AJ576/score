"""Mock-data loader for the stock JSON file at ``data/mock_stocks.json``.

This module is intentionally small and dependency-light: just the standard
library plus optional numpy.  It is fully implemented so the user can
exercise the pipeline end-to-end while the C++ math is still being filled
in.
"""

from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

from ._core import Series

# ---------------------------------------------------------------------------
# Path resolution
# ---------------------------------------------------------------------------
_PACKAGE_ROOT = Path(__file__).resolve().parent
_REPO_ROOT = _PACKAGE_ROOT.parent.parent
DEFAULT_DATA_PATH = _REPO_ROOT / "data" / "mock_stocks.json"

VALID_FIELDS = ("open", "high", "low", "close", "volume")


# ---------------------------------------------------------------------------
# Lightweight container
# ---------------------------------------------------------------------------
@dataclass(frozen=True)
class MockData:
    """Result of :func:`load_mock_data`.

    Attributes
    ----------
    metadata
        Top-level metadata block from the JSON file.
    bars
        Dict ``{ticker: list[bar]}`` where each ``bar`` is a dict with
        the keys date/open/high/low/close/volume.
    """

    metadata: dict[str, Any]
    bars: dict[str, list[dict[str, Any]]]

    @property
    def tickers(self) -> list[str]:
        return list(self.bars.keys())

    def __repr__(self) -> str:  # pragma: no cover - cosmetic
        return (
            f"<MockData tickers={self.tickers!r} "
            f"n_periods={self.metadata.get('n_periods')}>"
        )


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------
def load_mock_data(path: str | Path | None = None) -> MockData:
    """Load the mock JSON file and return a :class:`MockData`.

    Parameters
    ----------
    path
        Optional explicit path.  When omitted, the bundled
        ``data/mock_stocks.json`` is used.

    Raises
    ------
    FileNotFoundError
        If the file does not exist.
    json.JSONDecodeError
        If the file is not valid JSON.
    KeyError
        If the JSON does not have the expected ``metadata`` / ``data``
        top-level keys.
    """
    target = Path(path) if path is not None else DEFAULT_DATA_PATH
    with target.open("r", encoding="utf-8") as fh:
        raw = json.load(fh)

    if "metadata" not in raw or "data" not in raw:
        raise KeyError(
            f"{target} is missing required top-level keys "
            f"'metadata' and/or 'data'"
        )

    return MockData(metadata=raw["metadata"], bars=raw["data"])


def get_prices(
    data: MockData,
    ticker: str,
    field: str = "close",
) -> Series:
    """Extract a price series for ``ticker``.

    Parameters
    ----------
    data
        A :class:`MockData` returned by :func:`load_mock_data`.
    ticker
        The ticker symbol to extract (e.g. ``"AAPL"``).
    field
        Which field to extract: one of ``open``, ``high``, ``low``,
        ``close`` (default), or ``volume``.

    Returns
    -------
    Series
        A :class:`score.Series` with the price values and matching dates.
    """
    if field not in VALID_FIELDS:
        raise ValueError(
            f"field must be one of {VALID_FIELDS!r}, got {field!r}"
        )
    if ticker not in data.bars:
        raise KeyError(
            f"ticker {ticker!r} not in data; available: {data.tickers!r}"
        )

    bars = data.bars[ticker]
    values = [float(bar[field]) for bar in bars]
    dates = [str(bar["date"]) for bar in bars]
    return Series(values, dates)


def list_tickers(data: MockData) -> list[str]:
    """Return the available tickers in ``data``."""
    return data.tickers


def iter_bars(data: MockData, ticker: str) -> Iterable[dict[str, Any]]:
    """Iterate the OHLCV bars for ``ticker`` in chronological order."""
    if ticker not in data.bars:
        raise KeyError(ticker)
    return iter(data.bars[ticker])
