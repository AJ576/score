from abc import ABC, abstractmethod

from .clock import Clock
from .market import MarketData


class Asset(ABC):
    """Shared interface for the Composite pattern: a leaf (Stock) and a
    composite (Portfolio) both answer .value(), so callers treat one-or-many
    uniformly."""

    @abstractmethod
    def value(self):
        ...


class Stock(Asset):
    """A single holding (leaf). Has-a Clock and has-a MarketData provider,
    both injected so the stock never reaches out to globals."""

    def __init__(self, symbol, shares=1, clock=None, data=None):
        self.symbol = symbol
        self.shares = shares
        self.clock = clock or Clock()
        self.data = data or MarketData(symbol)

    def get_price(self):
        return self.data.last_close(intraday=self.clock.is_after_hours())

    def value(self):
        return self.get_price() * self.shares

    def get_open_price(self):
        return self.data.open()

    def get_high_price(self):
        return self.data.high()

    def get_low_price(self):
        return self.data.low()

    def get_info(self):
        return self.data.info

    def get_market_cap(self):
        return self.data.info["marketCap"]

    def get_pe_ratio(self):
        return self.data.info["trailingPE"]

    def get_dividend_yield(self):
        return self.data.info["dividendYield"]

    def get_eps(self):
        return self.data.info["trailingEps"]


class Portfolio(Asset):
    """A composite of Assets. Its value is the sum of its children's values,
    so a Portfolio is itself an Asset and can nest inside another Portfolio."""

    def __init__(self, name):
        self.name = name
        self.assets = {}

    def add(self, key, asset):
        self.assets[key] = asset
        return self

    def value(self):
        return sum(asset.value() for asset in self.assets.values())
