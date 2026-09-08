"""Runnable entry point: `uv run score` or `python -m score`."""

from score import Portfolio, Stock


def main():
    portfolio = Portfolio("My Portfolio")
    portfolio.add("AAPL", Stock("AAPL", shares=10))
    print(f"{portfolio.name}: ${portfolio.value():,.2f}")


if __name__ == "__main__":
    main()
