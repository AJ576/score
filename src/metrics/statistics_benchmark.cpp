#include <score/statistics/cdf.hpp>
#include <score/statistics/wiener_process.hpp>

#include <array>
#include <cmath>
#include <iostream>
#include <vector>

void validate_wiener(double check_time, double time_step, int n_paths) {
    const double expected_stddev = std::sqrt(check_time);

    std::vector<double> samples;
    samples.reserve(n_paths);

    for (int i = 0; i < n_paths; ++i) {
        auto path = wiener::wiener_process(check_time, time_step);
        samples.push_back(path.back());
    }

    // --- mean/variance check (unchanged from before) ---
    double sum = 0.0;
    for (double s : samples)
        sum += s;
    const double sample_mean = sum / samples.size();

    double sum_sq_dev = 0.0;
    for (double s : samples)
        sum_sq_dev += (s - sample_mean) * (s - sample_mean);
    const double sample_variance = sum_sq_dev / (samples.size() - 1);

    std::cout << "sample mean = " << sample_mean << " (expect ~0)\n";
    std::cout << "sample variance = " << sample_variance << " (expect ~" << check_time << ")\n";

    constexpr int n_bins = 10;
    std::array<int, n_bins> bin_counts{}; // zero-initialized

    for (double s : samples) {
        // Φ(s) under the theoretical distribution N(0, check_time)
        const double u =
            my_math::cdf(s, 0.0, expected_stddev); // should be ~Uniform(0,1) if s really is normal

        int bin = static_cast<int>(u * n_bins);
        if (bin == n_bins)
            bin -= 1; // guard u==1.0 edge case landing out of range
        bin_counts[bin]++;
    }

    const double expected_per_bin = static_cast<double>(n_paths) / n_bins;
    std::cout << "\nCDF-uniformity check (expect ~" << expected_per_bin << " per bin):\n";
    for (int i = 0; i < n_bins; ++i)
        std::cout << "  bin " << i << ": " << bin_counts[i] << "\n";
}

int main() {
    validate_wiener(/*check_time=*/14.0, /*time_step=**/ 0.05, /*n_paths=*/100000);
}
