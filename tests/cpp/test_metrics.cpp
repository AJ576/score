#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <score/core/types.hpp>
#include <score/metrics/stock_metrics.hpp>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using score::Series;
using score::metrics::StockAnalyzer;

// Most tests carry [!shouldfail] until the user implements the math.
// A simple constant-growth price path is used so every metric has a
// closed-form expected value.

TEST_CASE("StockAnalyzer::cumulative_return", "[metrics][cumret][!shouldfail]") {
    // P_0=100, ..., P_4=110.  cumret = 0.10
    Series<double> p({100.0, 102.5, 105.0, 107.5, 110.0});
    StockAnalyzer sa(p);
    REQUIRE_THAT(sa.cumulative_return(), WithinAbs(0.10, 1e-12));
}

TEST_CASE("StockAnalyzer::simple_returns length", "[metrics][returns][!shouldfail]") {
    Series<double> p({100.0, 110.0, 121.0});
    StockAnalyzer sa(p);
    auto r = sa.simple_returns();
    REQUIRE(r.size() == 2);
    REQUIRE_THAT(r[0], WithinAbs(0.10, 1e-12));
    REQUIRE_THAT(r[1], WithinAbs(0.10, 1e-12));
}

TEST_CASE("StockAnalyzer::log_returns length", "[metrics][returns][!shouldfail]") {
    Series<double> p({1.0, 2.0, 4.0});
    StockAnalyzer sa(p);
    auto r = sa.log_returns();
    REQUIRE(r.size() == 2);
    // log(2) twice
    REQUIRE_THAT(r[0], WithinAbs(0.6931471805599453, 1e-12));
    REQUIRE_THAT(r[1], WithinAbs(0.6931471805599453, 1e-12));
}

TEST_CASE("StockAnalyzer::max_drawdown on a decline", "[metrics][drawdown][!shouldfail]") {
    // Peak = 100 at t=2, then price drops to 60 → drawdown = 0.40
    Series<double> p({80.0, 90.0, 100.0, 80.0, 60.0, 70.0});
    StockAnalyzer sa(p);
    REQUIRE_THAT(sa.max_drawdown(), WithinAbs(0.40, 1e-12));
}

TEST_CASE("StockAnalyzer::max_drawdown on monotonically increasing prices",
          "[metrics][drawdown]") {
    // No `[!shouldfail]`: the answer is 0.0, which the stub also returns,
    // and 0.0 is still the correct answer once max_drawdown is implemented.
    Series<double> p({100.0, 101.0, 102.0, 103.0, 104.0});
    StockAnalyzer sa(p);
    REQUIRE_THAT(sa.max_drawdown(), WithinAbs(0.0, 1e-12));
}
