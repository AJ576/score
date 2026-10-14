#include <score/core/exceptions.hpp>
#include <score/core/types.hpp>
#include <score/statistics/descriptive.hpp>
#include <score/statistics/risk.hpp>

#include <cmath>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using Catch::Matchers::WithinAbs;
using score::Series;
using score::statistics::DescriptiveStats;

// Most tests carry [!shouldfail] until the user implements the C++
// math.  Drop the tag from a test as the corresponding method is filled
// in.

TEST_CASE("DescriptiveStats::mean on {1,2,3,4,5}", "[stats][mean]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0, 5.0});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.mean(), WithinAbs(3.0, 1e-12));
}

TEST_CASE("DescriptiveStats::variance sample on {1,2,3,4,5}", "[stats][variance]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0, 5.0});
    DescriptiveStats d(s);
    // sample variance = (Σ (x - 3)²) / (n - 1) = 10 / 4 = 2.5
    REQUIRE_THAT(d.variance(true), WithinAbs(2.5, 1e-12));
}

TEST_CASE("DescriptiveStats::variance population on {1,2,3,4,5}", "[stats][variance]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0, 5.0});
    DescriptiveStats d(s);
    // pop variance = 10 / 5 = 2.0
    REQUIRE_THAT(d.variance(false), WithinAbs(2.0, 1e-12));
}

TEST_CASE("DescriptiveStats::min/max", "[stats][minmax]") {
    Series<double> s({3.0, 1.0, 4.0, 1.5, 9.0, 2.6});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.min(), WithinAbs(1.0, 1e-12));
    REQUIRE_THAT(d.max(), WithinAbs(9.0, 1e-12));
}

TEST_CASE("DescriptiveStats::range", "[stats][range]") {
    Series<double> s({3.0, 1.0, 4.0, 1.5, 9.0, 2.6});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.range(), WithinAbs(8.0, 1e-12));
}

TEST_CASE("DescriptiveStats::median odd n", "[stats][median]") {
    Series<double> s({3.0, 1.0, 4.0, 1.5, 9.0});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.median(), WithinAbs(3.0, 1e-12));
}

TEST_CASE("DescriptiveStats::median even n", "[stats][median]") {
    Series<double> s({1.0, 2.0, 3.0, 4.0});
    DescriptiveStats d(s);
    REQUIRE_THAT(d.median(), WithinAbs(2.5, 1e-12));
}

TEST_CASE("risk::log_returns on {1, 2, 4}", "[stats][risk]") {
    std::vector<double> prices = {1.0, 2.0, 4.0};
    auto r = risk::log_returns(prices);
    REQUIRE(r.size() == 2);
    REQUIRE_THAT(r[0], WithinAbs(std::log(2.0), 1e-12));
    REQUIRE_THAT(r[1], WithinAbs(std::log(2.0), 1e-12));
}

TEST_CASE("risk::calculate_risk on constant prices {100, 100, 100}", "[stats][risk]") {
    std::vector<double> prices = {100.0, 100.0, 100.0};
    auto stats = risk::calculate_risk(prices);
    REQUIRE(stats.n == 2);
    REQUIRE_THAT(stats.mean_daily_return, WithinAbs(0.0, 1e-12));
    REQUIRE_THAT(stats.daily_std_return, WithinAbs(0.0, 1e-12));
}

TEST_CASE("risk::calculate_risk on moving prices", "[stats][risk]") {
    std::vector<double> prices = {100.0, 105.0, 102.0, 108.0};
    auto stats = risk::calculate_risk(prices);
    REQUIRE(stats.n == 3);
    REQUIRE(stats.daily_std_return > 0.0);
}

TEST_CASE("risk error handling on invalid inputs", "[stats][risk]") {
    // Empty prices
    REQUIRE_THROWS_AS(risk::log_returns({}), score::EmptySeriesError);

    // Fewer than 2 prices
    REQUIRE_THROWS_AS(risk::log_returns({100.0}), score::DomainError);

    // Non-positive price
    REQUIRE_THROWS_AS(risk::log_returns({100.0, -1.0}), score::DomainError);
    REQUIRE_THROWS_AS(risk::log_returns({0.0, 100.0}), score::DomainError);

    // Empty returns
    REQUIRE_THROWS_AS(risk::calculate_risk_from_returns({}), score::EmptySeriesError);

    // Fewer than 2 returns
    REQUIRE_THROWS_AS(risk::calculate_risk_from_returns({0.05}), score::DomainError);
}

