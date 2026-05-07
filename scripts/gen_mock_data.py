#!/usr/bin/env python3
"""Generate ``data/mock_stocks.json`` with deterministic OHLCV bars.

The price path follows a Geometric Brownian Motion (GBM) with per-ticker
drift and volatility:

    S_{t+1} = S_t * exp((mu - 0.5 * sigma^2) * dt + sigma * sqrt(dt) * Z)

where Z ~ N(0, 1).  Each daily bar's open/high/low/close are derived from
the close-to-close path with a small intraday wiggle so high >= close >=
low and high >= open >= low.  Volume is a positive integer drawn from a
log-normal distribution.

Run::

    python scripts/gen_mock_data.py

The output is fully deterministic given the seed in METADATA["seed"].
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass
from datetime import date, datetime, timedelta, timezone
from pathlib import Path

import numpy as np

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
N_PERIODS = 504  # ~ two trading years
SEED = 42
START_DATE = date(2024, 5, 7)
DT = 1.0 / 252  # one trading day in years
OUT_PATH = Path(__file__).resolve().parents[1] / "data" / "mock_stocks.json"


@dataclass(frozen=True)
class TickerConfig:
    symbol: str
    initial_price: float
    annual_drift: float       # mu, e.g. 0.10 = 10%
    annual_volatility: float  # sigma, e.g. 0.25 = 25%
    avg_daily_volume: float
    intraday_vol: float = 0.005  # rough intraday range as fraction of close


TICKERS = [
    TickerConfig("AAPL",  175.0, 0.12, 0.28, 6.5e7),
    TickerConfig("GOOGL", 165.0, 0.10, 0.30, 2.5e7),
    TickerConfig("MSFT",  410.0, 0.14, 0.24, 2.8e7),
    TickerConfig("AMZN",  180.0, 0.16, 0.34, 5.0e7),
    TickerConfig("TSLA",  200.0, 0.05, 0.55, 9.0e7),
]


# ---------------------------------------------------------------------------
# Generation
# ---------------------------------------------------------------------------
def _trading_days(start: date, n: int) -> list[date]:
    """Return ``n`` consecutive Mon-Fri dates starting on or after ``start``."""
    out: list[date] = []
    d = start
    while len(out) < n:
        if d.weekday() < 5:  # Mon..Fri
            out.append(d)
        d += timedelta(days=1)
    return out


def _gbm_path(cfg: TickerConfig, rng: np.random.Generator) -> np.ndarray:
    z = rng.standard_normal(N_PERIODS)
    drift = (cfg.annual_drift - 0.5 * cfg.annual_volatility ** 2) * DT
    diffusion = cfg.annual_volatility * math.sqrt(DT) * z
    log_increments = drift + diffusion
    log_path = np.concatenate(([0.0], np.cumsum(log_increments[:-1])))
    return cfg.initial_price * np.exp(log_path)


def _bars_for_ticker(
    cfg: TickerConfig,
    dates: list[date],
    rng: np.random.Generator,
) -> list[dict]:
    closes = _gbm_path(cfg, rng)
    # Open is the previous close plus a small overnight wiggle for t > 0;
    # for t == 0 we just use the initial price.
    opens = np.empty_like(closes)
    opens[0] = cfg.initial_price
    overnight = rng.normal(loc=0.0, scale=cfg.intraday_vol * 0.5, size=N_PERIODS - 1)
    opens[1:] = closes[:-1] * (1.0 + overnight)

    intraday_range = np.abs(rng.normal(loc=0.0, scale=cfg.intraday_vol, size=N_PERIODS))
    highs = np.maximum(opens, closes) * (1.0 + intraday_range)
    lows = np.minimum(opens, closes) * (1.0 - intraday_range)

    log_volumes = rng.normal(
        loc=math.log(cfg.avg_daily_volume),
        scale=0.25,
        size=N_PERIODS,
    )
    volumes = np.exp(log_volumes).astype(np.int64)

    bars: list[dict] = []
    for i, d in enumerate(dates):
        bars.append({
            "date":   d.isoformat(),
            "open":   round(float(opens[i]), 4),
            "high":   round(float(highs[i]), 4),
            "low":    round(float(lows[i]), 4),
            "close":  round(float(closes[i]), 4),
            "volume": int(volumes[i]),
        })
    return bars


def generate() -> dict:
    rng = np.random.default_rng(SEED)
    dates = _trading_days(START_DATE, N_PERIODS)

    data: dict[str, list[dict]] = {}
    for cfg in TICKERS:
        # Each ticker gets its own sub-generator from the master seed so
        # adding/removing a ticker does not perturb the others.
        sub_rng = np.random.default_rng(rng.integers(0, 2**31 - 1))
        data[cfg.symbol] = _bars_for_ticker(cfg, dates, sub_rng)

    metadata = {
        "generated_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "frequency":    "daily",
        "n_periods":    N_PERIODS,
        "tickers":      [cfg.symbol for cfg in TICKERS],
        "seed":         SEED,
        "start_date":   START_DATE.isoformat(),
        "model":        "GBM",
    }
    return {"metadata": metadata, "data": data}


def main() -> None:
    payload = generate()
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    with OUT_PATH.open("w", encoding="utf-8") as fh:
        json.dump(payload, fh, indent=2)
        fh.write("\n")
    n_tickers = len(payload["data"])
    n_bars = payload["metadata"]["n_periods"]
    print(f"Wrote {OUT_PATH} ({n_tickers} tickers x {n_bars} bars).")


if __name__ == "__main__":
    main()
