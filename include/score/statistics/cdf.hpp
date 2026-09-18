#pragma once

#include <array>
#include <cmath>

namespace naive {
/// Reference CDF via numerical (Simpson's rule) integration of normal_pdf.
/// Slow by design — used as a correctness baseline, not for production use.
double cdf(double x, double mean = 0.0, double stddev = 1.0, int n = 100000);
} // namespace naive

namespace std_math {
/// CDF via std::erfc(-z)/2.  Numerically preferred over cdf_erf for z << 0.
double cdf_erfc(double x, double mean = 0.0, double stddev = 1.0);

/// CDF via (1 + std::erf(z))/2.
double cdf_erf(double x, double mean = 0.0, double stddev = 1.0);
} // namespace std_math

// Header-only Cephes-style erf/erfc so callers (including Python bindings)
// can inline these hot, leaf math functions without relying on LTO.
namespace my_math {
namespace detail {

// erf(x), |x| <= 1: erf(x) = x * T(x^2) / U(x^2)
inline constexpr std::array<double, 5> T = {9.60497373987051638749E0, 9.00260197203842689217E1,
                                            2.23200534594684319226E3, 7.00332514112805075473E3,
                                            5.55923013010394962768E4};
inline constexpr std::array<double, 5> U = {3.35617141647503099647E1, 5.21357949780152679795E2,
                                            4.59432382970980127987E3, 2.26290000613890934246E4,
                                            4.92673942608635921086E4};

// erfc(x)*exp(x^2), 1 <= x < 8: erfc(x) = exp(-x^2) * P(x) / Q(x)
inline constexpr std::array<double, 9> P = {
    2.46196981473530512524E-10, 5.64189564831068821977E-1, 7.46321056442269912687E0,
    4.86371970985681366614E1,   1.96520832956077098242E2,  5.26445194995477358631E2,
    9.34528527171957607540E2,   1.02755188689515710272E3,  5.57535335369399327526E2};
inline constexpr std::array<double, 8> Q = {1.32281951154744992508E1, 8.67072140885989742329E1,
                                            3.54937778887819891062E2, 9.75708501743205489753E2,
                                            1.82390916687909736289E3, 2.24633760818710981792E3,
                                            1.65666309194161350182E3, 5.57535340817727675546E2};

// erfc(x)*exp(x^2), x >= 8: erfc(x) = exp(-x^2) * R(x) / S(x)
inline constexpr std::array<double, 6> R = {5.64189583547755073984E-1, 1.27536670759978104416E0,
                                            5.01905042251180477414E0,  6.16021097993053585195E0,
                                            7.40974269950448939160E0,  2.97886665372100240670E0};
inline constexpr std::array<double, 6> S = {2.26052863220117276590E0, 9.39603524938001434673E0,
                                            1.20489539808096656605E1, 1.70814450747565897222E1,
                                            9.60896809063285878198E0, 3.36907645100081516050E0};

inline constexpr double kMaxLog = 7.09782712893383996843E2; // ln(DBL_MAX), underflow guard

// Non-monic polynomial evaluation: coeff[0]*x^N + ... + coeff[N].
template <std::size_t N>
constexpr double polevl(double x, const std::array<double, N>& coeff) noexcept {
    double ans = coeff[0];
    for (std::size_t i = 1; i < N; ++i) {
        ans = std::fma(ans, x, coeff[i]);
    }
    return ans;
}

// Monic polynomial evaluation (implicit leading coefficient of 1).
template <std::size_t N>
constexpr double p1evl(double x, const std::array<double, N>& coeff) noexcept {
    double ans = x + coeff[0];
    for (std::size_t i = 1; i < N; ++i) {
        ans = std::fma(ans, x, coeff[i]);
    }
    return ans;
}

} // namespace detail

[[nodiscard]] inline double my_erfc(double a) noexcept;

[[nodiscard]] inline double my_erf(double x) noexcept {
    if (std::fabs(x) > 1.0) {
        return 1.0 - my_erfc(x);
    }
    const double x2 = x * x;
    return x * detail::polevl(x2, detail::T) / detail::p1evl(x2, detail::U);
}

[[nodiscard]] inline double my_erfc(double a) noexcept {
    const double x = std::fabs(a);
    if (x < 1.0) {
        return 1.0 - my_erf(a);
    }

    const double z = -a * a;
    if (z < -detail::kMaxLog) {
        return (a < 0) ? 2.0 : 0.0; // underflow: result indistinguishable from the limit
    }

    // Deliberate simplification vs. Cephes's precision-guarded expx2(): plain
    // exp() of x*x loses a little precision but is cheaper and precise enough here.
    const double exp_z = std::exp(z);
    const auto [p, q] =
        x < 8.0 ? std::pair{detail::polevl(x, detail::P), detail::p1evl(x, detail::Q)}
                : std::pair{detail::polevl(x, detail::R),
                            detail::p1evl(x, detail::S)}; // its better to have p and q as a pair,
                                                          // instead of their own variables

    const double y = (exp_z * p) / q;
    return (a < 0) ? 2.0 - y : y;
}

[[nodiscard]] inline double cdf(double x, double mean = 0.0, double stddev = 1.0) noexcept {
    const double z = (x - mean) / (stddev * std::sqrt(2.0));
    return 0.5 * my_erfc(-z);
}

} // namespace my_math
