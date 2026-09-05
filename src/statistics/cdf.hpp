#pragma once
namespace naive
{
    double cdf(double x,
               double mean = 0.0,
               double stddev = 1.0,
               int n = 100000);
}
namespace std_math
{
    double cdf_erfc(double x, double mean = 0.0, double stddev = 1.0);
    double cdf_erf(double x, double mean = 0.0, double stddev = 1.0);
}

namespace my_math
{
    double my_erf(double x);
    double my_erfc(double x);
    double cdf(double x, double mean = 0.0, double stddev = 1.0);
} // namespace my_math
