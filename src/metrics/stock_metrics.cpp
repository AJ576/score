#include <score/core/exceptions.hpp>
#include <score/metrics/stock_metrics.hpp>
#include <score/statistics/descriptive.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace score::metrics {

StockAnalyzer::StockAnalyzer(const Series<double>& prices) : prices_(prices) {}

Series<double> StockAnalyzer::simple_returns() const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::simple_returns");
    }
    if (prices_.size() < 2) {
        throw DomainError("StockAnalyzer::simple_returns: prices must have at least 2 elements");
    }
    std::vector<double> data;
    data.reserve(prices_.size() - 1);
    for (std::size_t i = 1; i < prices_.size(); ++i) {
        data.push_back(prices_[i] / prices_[i - 1] - 1.0);
    }
    return Series<double>(std::move(data));
}

Series<double> StockAnalyzer::log_returns() const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::log_returns");
    }
    if (prices_.size() < 2) {
        throw DomainError("StockAnalyzer::log_returns: prices must have at least 2 elements");
    }
    std::vector<double> data;
    data.reserve(prices_.size() - 1);
    for (std::size_t i = 1; i < prices_.size(); ++i) {
        data.push_back(std::log(prices_[i] / prices_[i - 1]));
    }
    return Series<double>(std::move(data));
}

double StockAnalyzer::cumulative_return() const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::cumulative_return");
    }
    return (prices_[prices_.size() - 1] / prices_[0]) - 1.0;
}

double StockAnalyzer::annualized_return(int periods_per_year) const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::annualized_return");
    }
    const auto n_periods = prices_.size() - 1;
    return std::pow(1.0 + cumulative_return(),
                    static_cast<double>(periods_per_year) / static_cast<double>(n_periods)) -
           1.0;
}

double StockAnalyzer::annualized_volatility(int periods_per_year) const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::annualized_volatility");
    }
    const auto returns = simple_returns();
    return statistics::DescriptiveStats(returns).stddev() *
           std::sqrt(static_cast<double>(periods_per_year));
}

double StockAnalyzer::sharpe_ratio(double risk_free_rate, int periods_per_year) const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::sharpe_ratio");
    }
    return (annualized_return(periods_per_year) - risk_free_rate) /
           annualized_volatility(periods_per_year);
}

double StockAnalyzer::max_drawdown() const {
    if (prices_.empty()) {
        throw EmptySeriesError("StockAnalyzer::max_drawdown");
    }
    double max_dd = 0.0;
    double peak = prices_[0];
    for (std::size_t i = 1; i < prices_.size(); ++i) {
        if (prices_[i] > peak) {
            peak = prices_[i];
        }
        const double dd = (peak - prices_[i]) / peak;
        max_dd = std::max(max_dd, dd);
    }
    return max_dd;
}

} // namespace score::metrics
