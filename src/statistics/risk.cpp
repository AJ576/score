#include <score/statistics/risk.hpp>
#include <score/core/exceptions.hpp>

#include <cmath>
#include <cstddef>
#include <numeric>
#include <string>
#include <vector>

namespace risk
{
    std::vector<double> log_returns(const std::vector<double>& prices)
    {
        if (prices.empty())
        {
            throw score::EmptySeriesError("risk::log_returns: prices series is empty");
        }

        if (prices.size() < 2)
        {
            throw score::DomainError(
                "risk::log_returns requires at least 2 prices, got " + std::to_string(prices.size()));
        }

        if (prices[0] <= 0.0)
        {
            throw score::DomainError(
                "risk::log_returns: price at index 0 must be positive, got " + std::to_string(prices[0]));
        }

        std::vector<double> returns;
        returns.reserve(prices.size() - 1);
        for (std::size_t i = 1; i < prices.size(); i++)
        {
            if (prices[i] <= 0.0)
            {
                throw score::DomainError(
                    "risk::log_returns: price at index " + std::to_string(i) + " must be positive, got " +
                    std::to_string(prices[i]));
            }
            returns.push_back(std::log(prices[i] / prices[i - 1]));
        }
        return returns;
    }

    RiskStats calculate_risk_from_returns(const std::vector<double>& returns)
    {
        if (returns.empty())
        {
            throw score::EmptySeriesError("risk::calculate_risk_from_returns: returns series is empty");
        }

        if (returns.size() < 2)
        {
            throw score::DomainError(
                "risk::calculate_risk requires at least 2 returns to compute sample variance, got " +
                std::to_string(returns.size()));
        }

        RiskStats stats{};
        stats.n = returns.size();

        const double sum = std::accumulate(returns.begin(), returns.end(), 0.0);
        stats.mean_daily_return = sum / static_cast<double>(stats.n);

        double sum_sq = 0.0;
        for (double r : returns)
        {
            const double diff = r - stats.mean_daily_return;
            sum_sq += diff * diff;
        }
        stats.daily_std_return = std::sqrt(sum_sq / static_cast<double>(stats.n - 1));

        return stats;
    }

    RiskStats calculate_risk(const std::vector<double>& prices)
    {
        return calculate_risk_from_returns(log_returns(prices));
    }
}