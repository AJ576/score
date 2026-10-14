import yfinance as yf

data = yf.download("AAPL", start="2020-01-01", end="2025-01-01")
data = data["Close"].reset_index()  # just date + close, single clean header
data.columns = ["date", "close"]
data.to_csv("aapl_prices.csv", index=False)