#include <score/core/exceptions.hpp>
#include <score/statistics/descriptive.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

namespace score::statistics {

DescriptiveStats::DescriptiveStats(const Series<double>& series) : series_(series) {}

double DescriptiveStats::mean() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::mean");
    }
    return std::accumulate(series_.cbegin(), series_.cend(), 0.0) /
           static_cast<double>(series_.size());
}

double DescriptiveStats::variance(bool sample) const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::variance");
    }
    const auto n = series_.size();
    const double m = mean();
    const double sum_sq = std::accumulate(series_.cbegin(), series_.cend(), 0.0,
                                          [m](double sum, double value) {
                                              const double d = value - m;
                                              return sum + d * d;
                                          });
    const double denom = sample ? static_cast<double>(n - 1) : static_cast<double>(n);
    return sum_sq / denom;
}

double DescriptiveStats::stddev(bool sample) const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::stddev");
    }
    return std::sqrt(variance(sample));
}

double DescriptiveStats::median() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::median");
    }
    std::vector<double> values(series_.cbegin(), series_.cend());
    const auto mid = values.size() / 2;
    std::nth_element(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(mid),
                     values.end());
    if (values.size() % 2 == 0) {
        const double upper = values[mid];
        const double lower =
            *std::max_element(values.begin(), values.begin() + static_cast<std::ptrdiff_t>(mid));
        return (lower + upper) / 2.0;
    }
    return values[mid];
}

double DescriptiveStats::min() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::min");
    }
    return *std::min_element(series_.cbegin(), series_.cend());
}

double DescriptiveStats::max() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::max");
    }
    return *std::max_element(series_.cbegin(), series_.cend());
}

double DescriptiveStats::range() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::range");
    }
    return max() - min();
}

double DescriptiveStats::quantile(double q) const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::quantile");
    }
    if (q < 0 || q > 1) {
        throw DomainError("q must be in [0, 1]");
    }
    std::vector<double> values(series_.cbegin(), series_.cend());
    std::sort(values.begin(), values.end());
    const double pos = q * static_cast<double>(values.size() - 1);
    const auto lo = static_cast<std::size_t>(pos);
    const auto hi = lo + 1;
    if (hi >= values.size()) {
        return values.back();
    }
    const double frac = pos - static_cast<double>(lo);
    return values[lo] + frac * (values[hi] - values[lo]);
}

double DescriptiveStats::skewness() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::skewness");
    }
    const auto n = series_.size();
    const double m = mean();
    const double s = stddev(true);
    const double sum_cubed = std::accumulate(series_.cbegin(), series_.cend(), 0.0,
                                              [m](double sum, double value) {
                                                  const double d = value - m;
                                                  return sum + d * d * d;
                                              });
    return sum_cubed /
           (static_cast<double>(n) * s * s * s);
}

double DescriptiveStats::kurtosis() const {
    if (series_.empty()) {
        throw EmptySeriesError("DescriptiveStats::kurtosis");
    }
    const auto n = series_.size();
    const double m = mean();
    const double s = stddev(true);
    const double sum_quartic = std::accumulate(series_.cbegin(), series_.cend(), 0.0,
                                                [m](double sum, double value) {
                                                    const double d = value - m;
                                                    return sum + d * d * d * d;
                                                });
    return sum_quartic /
               (static_cast<double>(n) * s * s * s * s) -
           3.0;
}

} // namespace score::statistics
