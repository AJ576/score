#pragma once

#include <cstddef>
#include <vector>

namespace risk {

struct RiskStats {
    double mean_daily_return;
    double daily_std_return;
    std::size_t n;
};

std::vector<double> log_returns(const std::vector<double>& prices);

RiskStats calculate_risk(const std::vector<double>& prices);

RiskStats calculate_risk_from_returns(const std::vector<double>& returns);

} // namespace risk
