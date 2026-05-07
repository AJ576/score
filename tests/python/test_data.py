"""Tests for the JSON mock-data loader.

These pass once ``data/mock_stocks.json`` has been generated; no native
extension is required.
"""

from __future__ import annotations

from pathlib import Path

import pytest

from score.data import (
    DEFAULT_DATA_PATH,
    VALID_FIELDS,
    MockData,
    get_prices,
    load_mock_data,
)

EXPECTED_TICKERS = {"AAPL", "GOOGL", "MSFT", "AMZN", "TSLA"}


@pytest.fixture(scope="module")
def loaded(data_path: Path) -> MockData:
    if not data_path.exists():
        pytest.skip(
            f"{data_path} not found; run `python scripts/gen_mock_data.py` first."
        )
    return load_mock_data(data_path)


def test_default_path_resolves_to_data_dir() -> None:
    assert DEFAULT_DATA_PATH.name == "mock_stocks.json"
    assert DEFAULT_DATA_PATH.parent.name == "data"


def test_load_mock_data_returns_mock_data(loaded: MockData) -> None:
    assert isinstance(loaded, MockData)


def test_metadata_present(loaded: MockData) -> None:
    md = loaded.metadata
    assert "frequency" in md
    assert "n_periods" in md
    assert "tickers" in md


def test_all_expected_tickers_present(loaded: MockData) -> None:
    assert EXPECTED_TICKERS.issubset(set(loaded.tickers))


def test_per_ticker_bar_count_matches_metadata(loaded: MockData) -> None:
    n = int(loaded.metadata["n_periods"])
    for ticker in loaded.tickers:
        assert len(loaded.bars[ticker]) == n


def test_get_prices_default_field(loaded: MockData) -> None:
    p = get_prices(loaded, "AAPL")
    assert len(p) == int(loaded.metadata["n_periods"])
    assert all(v > 0 for v in p)


@pytest.mark.parametrize("field", VALID_FIELDS)
def test_get_prices_all_fields(loaded: MockData, field: str) -> None:
    p = get_prices(loaded, "AAPL", field=field)
    assert len(p) > 0


def test_get_prices_unknown_field(loaded: MockData) -> None:
    with pytest.raises(ValueError):
        get_prices(loaded, "AAPL", field="not_a_field")


def test_get_prices_unknown_ticker(loaded: MockData) -> None:
    with pytest.raises(KeyError):
        get_prices(loaded, "ZZZZ")
