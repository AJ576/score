#include <catch2/catch_test_macros.hpp>

#include <score/c_kernels/kernels.h>

#include <array>
#include <cmath>
#include <limits>

// These tests are tagged [!shouldfail] because the kernel bodies are
// stubs that return 0.0.  As you fill in the kernels, flip the tag
// off (delete "[!shouldfail]") and the tests will start passing.

TEST_CASE("score_k_sum on a small known input", "[kernels][sum][!shouldfail]") {
    std::array<double, 4> xs = {1.0, 2.0, 3.0, 4.0};
    REQUIRE(score_k_sum(xs.data(), xs.size()) == 10.0);
}

TEST_CASE("score_k_sum on empty returns 0", "[kernels][sum]") {
    REQUIRE(score_k_sum(nullptr, 0) == 0.0);
}

TEST_CASE("score_k_dot on small known input", "[kernels][dot][!shouldfail]") {
    std::array<double, 3> xs = {1.0, 2.0, 3.0};
    std::array<double, 3> ys = {4.0, 5.0, 6.0};
    // 1*4 + 2*5 + 3*6 = 32
    REQUIRE(score_k_dot(xs.data(), ys.data(), xs.size()) == 32.0);
}

TEST_CASE("score_k_min on a small input", "[kernels][min][!shouldfail]") {
    std::array<double, 5> xs = {3.0, 1.0, 4.0, 1.5, 9.0};
    REQUIRE(score_k_min(xs.data(), xs.size()) == 1.0);
}

TEST_CASE("score_k_min on empty returns +inf", "[kernels][min]") {
    REQUIRE(std::isinf(score_k_min(nullptr, 0)));
    REQUIRE(score_k_min(nullptr, 0) > 0);
}

TEST_CASE("score_k_max on a small input", "[kernels][max][!shouldfail]") {
    std::array<double, 5> xs = {3.0, 1.0, 4.0, 1.5, 9.0};
    REQUIRE(score_k_max(xs.data(), xs.size()) == 9.0);
}

TEST_CASE("score_k_max on empty returns -inf", "[kernels][max]") {
    REQUIRE(std::isinf(score_k_max(nullptr, 0)));
    REQUIRE(score_k_max(nullptr, 0) < 0);
}

TEST_CASE("score_k_accumulate_squared_dev on known input",
          "[kernels][var][!shouldfail]") {
    std::array<double, 4> xs = {1.0, 2.0, 3.0, 4.0};
    // mean = 2.5
    // squared devs = 2.25 + 0.25 + 0.25 + 2.25 = 5.0
    REQUIRE(score_k_accumulate_squared_dev(xs.data(), xs.size(), 2.5) == 5.0);
}
