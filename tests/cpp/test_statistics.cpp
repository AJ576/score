#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <score/core/types.hpp>
#include <score/statistics/descriptive.hpp>

using Catch::Matchers::WithinAbs;
using score::Series;
using score::statistics::DescriptiveStats;

// Most tests carry [!shouldfail] until the user implements the C++
// math.  Drop the tag from a test as the corresponding method is filled
// in.

TEST_CASE("DescriptiveStats::mean on {1,2,3,4,5}", "[stats][mean][!shouldfail]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0, 5.0});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.mean(), WithinAbs(3.0, 1e-12));
}

TEST_CASE("DescriptiveStats::variance sample on {1,2,3,4,5}",
          "[stats][variance][!shouldfail]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0, 5.0});
    DescriptiveStats d(s);
    // sample variance = (Σ (x - 3)²) / (n - 1) = 10 / 4 = 2.5
    REQUIRE_THAT(d.variance(true), WithinAbs(2.5, 1e-12));
}

TEST_CASE("DescriptiveStats::variance population on {1,2,3,4,5}",
          "[stats][variance][!shouldfail]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0, 5.0});
    DescriptiveStats d(s);
    // pop variance = 10 / 5 = 2.0
    REQUIRE_THAT(d.variance(false), WithinAbs(2.0, 1e-12));
}

TEST_CASE("DescriptiveStats::min/max", "[stats][minmax][!shouldfail]") {
    Series<double> s({3.0, 1.0, 4.0, 1.5, 9.0, 2.6});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.min(), WithinAbs(1.0, 1e-12));
    REQUIRE_THAT(d.max(), WithinAbs(9.0, 1e-12));
}

TEST_CASE("DescriptiveStats::range", "[stats][range][!shouldfail]") {
    Series<double> s({3.0, 1.0, 4.0, 1.5, 9.0, 2.6});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.range(), WithinAbs(8.0, 1e-12));
}

TEST_CASE("DescriptiveStats::median odd n", "[stats][median][!shouldfail]") {
    Series<double> s({3.0, 1.0, 4.0, 1.5, 9.0});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.median(), WithinAbs(3.0, 1e-12));
}

TEST_CASE("DescriptiveStats::median even n", "[stats][median][!shouldfail]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.median(), WithinAbs(2.5, 1e-12));
}
