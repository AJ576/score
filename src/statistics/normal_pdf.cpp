#include <score/statistics/normal_pdf.hpp>

#include <cmath>

double normal_pdf(double x, double mean, double stddev) {
    const double z = (x - mean) / stddev;
    const double coeff = 1.0 / (stddev * std::sqrt(2.0 * M_PI));
    return coeff * std::exp(-0.5 * z * z);
}