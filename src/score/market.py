from functools import cached_property

import yfinance as yf


class MarketData:
    """Facade over the yfinance API. Owns the single daily-history fetch and caches it."""

    def __init__(self, symbol, ticker=None):
        self._ticker = ticker or yf.Ticker(symbol)

    @cached_property
    def _daily(self):
        return self._ticker.history(period="1d")

    @cached_property
    def _intraday(self):
        return self._ticker.history(period="1d", interval="1m")

    @cached_property
    def info(self):
        return self._ticker.info

    def last_close(self, intraday):
        frame = self._intraday if intraday else self._daily
        return frame["Close"].iloc[-1]

    def open(self):
        return self._daily["Open"].iloc[0]

    def high(self):
        return self._daily["High"].iloc[0]

    def low(self):xp
        return self._daily["Low"].iloc[0]
