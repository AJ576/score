#pragma once

#include <score/core/types.hpp>

namespace score::metrics {

/// StockAnalyzer: derived financial metrics over a price series.
///
/// Conventions:
///   * `prices` is interpreted as a sequence of close prices on a uniform
///     grid (most commonly daily).
///   * `periods_per_year` is the annualization factor.  Daily prices use
///     252; weekly uses 52; monthly uses 12.
///   * Returns are unitless fractions (0.05 == 5%).
class StockAnalyzer {
public:
    explicit StockAnalyzer(const Series<double>& prices);

    /// Simple returns:
    ///   r_t = (P_t / P_{t-1}) - 1
    /// Output length is prices.size() - 1.
    [[nodiscard]] Series<double> simple_returns() const;

    /// Log returns:
    ///   r_t = ln(P_t / P_{t-1})
    /// Output length is prices.size() - 1.
    [[nodiscard]] Series<double> log_returns() const;

    /// Cumulative return over the whole window:
    ///   (P_end / P_0) - 1
    [[nodiscard]] double cumulative_return() const;

    /// Annualized return, geometric:
    ///   (1 + cumulative_return)^(periods_per_year / n_periods) - 1
    [[nodiscard]] double annualized_return(int periods_per_year = 252) const;

    /// Annualized volatility:
    ///   stddev(simple_returns) * sqrt(periods_per_year)
    [[nodiscard]] double annualized_volatility(int periods_per_year = 252) const;

    /// Sharpe ratio:
    ///   (annualized_return - risk_free_rate) / annualized_volatility
    [[nodiscard]] double sharpe_ratio(double risk_free_rate = 0.0,
                                      int periods_per_year   = 252) const;

    /// Maximum drawdown.
    ///   max over t of (peak_until_t - P_t) / peak_until_t,
    /// returned as a non-negative fraction.  0 means no drawdown.
    [[nodiscard]] double max_drawdown() const;

private:
    const Series<double>& prices_;
};

}  // namespace score::metrics
