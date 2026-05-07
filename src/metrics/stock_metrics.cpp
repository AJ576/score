#include <score/metrics/stock_metrics.hpp>

#include <score/core/exceptions.hpp>
#include <score/statistics/descriptive.hpp>

namespace score::metrics {

StockAnalyzer::StockAnalyzer(const Series<double>& prices) : prices_(prices) {}

Series<double> StockAnalyzer::simple_returns() const {
    // TODO: r_t = P_t / P_{t-1} - 1, for t in [1, n)
    // Result has length n - 1.  Throw EmptySeriesError or DomainError on
    // sizes < 2.
    return Series<double>{};
}

Series<double> StockAnalyzer::log_returns() const {
    // TODO: r_t = ln(P_t / P_{t-1})
    // Watch for non-positive prices (throw DomainError).
    return Series<double>{};
}

double StockAnalyzer::cumulative_return() const {
    // TODO: (P_end / P_0) - 1
    return 0.0;
}

double StockAnalyzer::annualized_return(int /*periods_per_year*/) const {
    // TODO: (1 + cumulative_return)^(periods_per_year / n_periods) - 1
    // n_periods is prices_.size() - 1.
    return 0.0;
}

double StockAnalyzer::annualized_volatility(int /*periods_per_year*/) const {
    // TODO: stddev(simple_returns) * sqrt(periods_per_year)
    // Hint: build a DescriptiveStats over simple_returns().
    return 0.0;
}

double StockAnalyzer::sharpe_ratio(double /*risk_free_rate*/, int /*periods_per_year*/) const {
    // TODO: (annualized_return - rf) / annualized_volatility
    // Decide what to do when annualized_volatility == 0.
    return 0.0;
}

double StockAnalyzer::max_drawdown() const {
    // TODO: walk prices, track running peak, compute (peak - P_t)/peak,
    // return the maximum.  Result is a non-negative fraction.
    return 0.0;
}

}  // namespace score::metrics
