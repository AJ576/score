"""Fetch historical stock price data (hourly or daily) using yfinance.

Yahoo Finance limits hourly data (interval="1h") to the most recent 730 days (~2 years).
"""

from __future__ import annotations

import argparse
from pathlib import Path
import pandas as pd
import yfinance as yf


def fetch_hourly_data(
    ticker: str = "AAPL",
    period: str = "730d",
    output_path: str | Path | None = "aapl_hourly.csv",
    detect_overnight_gaps: bool = True,
) -> pd.DataFrame:
    """Download hourly bar data for a given ticker and save to CSV.

    Parameters
    ----------
    ticker : str
        Stock ticker symbol (e.g., 'AAPL', 'MSFT', 'SPY').
    period : str
        Lookback window. For 1h interval, Yahoo Finance allows a maximum of 730d (~2 years).
    output_path : str | Path | None
        Optional path to write the resulting CSV. If None, does not write to disk.
    detect_overnight_gaps : bool
        If True, adds an 'is_overnight_gap' boolean flag indicating whether the bar
        is the first bar after market open (crossing the overnight close-to-open window).

    Returns
    -------
    pd.DataFrame
        DataFrame with columns ['datetime', 'close'] (and optionally 'is_overnight_gap').
    """
    print(f"Fetching hourly data for {ticker} (period: {period}, interval: 1h)...")

    # Using yf.Ticker(...).history avoids MultiIndex columns that yf.download often returns
    t = yf.Ticker(ticker)
    df = t.history(period=period, interval="1h")

    if df.empty:
        raise ValueError(
            f"No data returned for ticker '{ticker}' with period='{period}'. "
            "Note: Yahoo Finance allows at most 730 days for 1h interval data."
        )

    # Standardize column naming and extract close prices
    df = df.reset_index()

    # The index column name is typically 'Datetime' for intraday history
    date_col = "Datetime" if "Datetime" in df.columns else "Date"

    # Convert timestamps to timezone-naive local/market format (or ISO string)
    if pd.api.types.is_datetime64_any_dtype(df[date_col]):
        # Keep consistent New York market timestamps without tz offset clutter
        try:
            df["datetime"] = df[date_col].dt.tz_convert("America/New_York").dt.strftime("%Y-%m-%d %H:%M:%S")
        except TypeError:
            # Already tz-naive
            df["datetime"] = df[date_col].dt.strftime("%Y-%m-%d %H:%M:%S")
    else:
        df["datetime"] = df[date_col].astype(str)

    df["close"] = df["Close"].astype(float)

    result_cols = ["datetime", "close"]

    if detect_overnight_gaps:
        # Check if the current timestamp belongs to a different calendar day than the previous bar
        raw_dates = pd.to_datetime(df[date_col])
        day_diff = raw_dates.dt.date != raw_dates.dt.date.shift(1)
        # The very first row is the start of the dataset, not a gap
        df["is_overnight_gap"] = day_diff
        df.loc[0, "is_overnight_gap"] = False
        result_cols.append("is_overnight_gap")

    result = df[result_cols].copy()

    if output_path is not None:
        out = Path(output_path)
        result.to_csv(out, index=False)
        print(f"Saved {len(result)} hourly bars to {out.resolve()}")

    return result


def main() -> None:
    parser = argparse.ArgumentParser(description="Fetch hourly stock data from Yahoo Finance.")
    parser.add_argument("--ticker", "-t", default="AAPL", help="Stock ticker symbol (default: AAPL)")
    parser.add_argument(
        "--period", "-p", default="730d", help="Data lookback period (max 730d for 1h interval, default: 730d)"
    )
    parser.add_argument(
        "--output", "-o", default="aapl_hourly.csv", help="Output CSV path (default: aapl_hourly.csv)"
    )
    parser.add_argument(
        "--no-gaps", action="store_true", help="Do not include the overnight gap detection column"
    )

    args = parser.parse_args()

    fetch_hourly_data(
        ticker=args.ticker,
        period=args.period,
        output_path=args.output,
        detect_overnight_gaps=not args.no_gaps,
    )


if __name__ == "__main__":
    main()
