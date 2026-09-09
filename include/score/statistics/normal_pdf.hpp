#pragma once

/// Standard/normal probability density function.
///   f(x) = 1/(σ√(2π)) · exp(-(x-μ)²/(2σ²))
double normal_pdf(double x, double mean = 0.0, double stddev = 1.0);
