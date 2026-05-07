#include <score/statistics/descriptive.hpp>

#include <score/core/exceptions.hpp>
#include <score/c_kernels/kernels.h>

namespace score::statistics {

DescriptiveStats::DescriptiveStats(const Series<double>& series) : series_(series) {}

double DescriptiveStats::mean() const {
    // TODO: arithmetic mean.
    //   μ = (1/n) Σ x_i
    // Hint: use score_k_sum(series_.data(), series_.size()) / n.
    // Throw EmptySeriesError when series_.empty().
    return 0.0;
}

double DescriptiveStats::variance(bool /*sample*/) const {
    // TODO: variance.
    //   sample:     s² = (1/(n-1)) Σ (x_i - μ)²
    //   population: σ² = (1/n)     Σ (x_i - μ)²
    // Hint: m = mean(); use score_k_accumulate_squared_dev(data, n, m).
    throw NotImplementedError("DescriptiveStats::variance");
}

double DescriptiveStats::stddev(bool /*sample*/) const {
    // TODO: std::sqrt(variance(sample))
    throw NotImplementedError("DescriptiveStats::stddev");
}

double DescriptiveStats::median() const {
    // TODO: 50th percentile.
    // Hint: copy values, std::nth_element, average two middles for even n.
    throw NotImplementedError("DescriptiveStats::median");
}

double DescriptiveStats::min() const {
    // TODO: score_k_min(series_.data(), series_.size())
    return 0.0;
}

double DescriptiveStats::max() const {
    // TODO: score_k_max(series_.data(), series_.size())
    return 0.0;
}

double DescriptiveStats::range() const {
    // TODO: max() - min()
    return 0.0;
}

double DescriptiveStats::quantile(double /*q*/) const {
    // TODO: q-quantile via linear interpolation between order statistics.
    // Validate q in [0, 1] (throw DomainError otherwise).
    throw NotImplementedError("DescriptiveStats::quantile");
}

double DescriptiveStats::skewness() const {
    // TODO: sample skewness.
    //   γ_1 = (1/n) Σ ((x_i - μ)/σ)³
    throw NotImplementedError("DescriptiveStats::skewness");
}

double DescriptiveStats::kurtosis() const {
    // TODO: excess kurtosis.
    //   γ_2 = (1/n) Σ ((x_i - μ)/σ)^4 - 3
    throw NotImplementedError("DescriptiveStats::kurtosis");
}

}  // namespace score::statistics
