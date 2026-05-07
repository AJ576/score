"""Shared pytest fixtures for the Python test-suite."""

from __future__ import annotations

from pathlib import Path

import pytest


@pytest.fixture(scope="session")
def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


@pytest.fixture(scope="session")
def data_path(repo_root: Path) -> Path:
    return repo_root / "data" / "mock_stocks.json"


@pytest.fixture()
def small_floats() -> list[float]:
    return [1.0, 2.0, 3.0, 4.0, 5.0]


@pytest.fixture()
def known_prices() -> list[float]:
    """A short, deterministic price path used across several tests."""
    return [100.0, 102.5, 105.0, 107.5, 110.0]
