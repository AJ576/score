#include <chrono>
#include <concepts>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../statistics/cdf.hpp"

template <std::invocable<double> F>
void run_benchmark(const std::string& name, F&& f, const std::vector<double>& inputs,
                   int iterations) {
    constexpr int warmup_iterations = 1000;

    for (double x : inputs) {

        volatile double sink = 0.0; // stops the compiler discarding the loop

        // warm-up: pay cache/branch-predictor cold-start cost before timing
        for (int i = 0; i < warmup_iterations; i++)
            // perturb slightly so the compiler can't collapse N identical calls into 1
            sink = f(x + i * 1e-15); // technincally overwrites but averaging out is just gonna lose
                                     // precision so whatever.

        // now start
        const auto start = std::chrono::steady_clock::now();
        for (int i = 0; i < iterations; ++i)
            sink = f(x + i * 1e-15);
        const auto end = std::chrono::steady_clock::now();

        const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        const double time_per_call = static_cast<double>(elapsed.count()) / iterations;

        std::cout << std::setprecision(17);
        std::cout << name << " | x = " << x << " | CDF ~= " << sink << " | time = " << time_per_call
                  << " ns\n";
    }
}

int main() {
    const std::vector<double> inputs = {-37,   -30, -15,  -10, -8.5, -7.5, -5, -2,  -1,  -0.5,
                                        -0.05, 0,   0.05, 0.5, 1,    2,    5,  7.5, 8.5, 10};

    run_benchmark("naive_simpson", [](double x) { return naive::cdf(x); }, inputs, 200);
    run_benchmark("erf_based", [](double x) { return std_math::cdf_erf(x); }, inputs, 1'000'000);
    run_benchmark("erfc_based", [](double x) { return std_math::cdf_erfc(x); }, inputs, 1'000'000);
    run_benchmark("cephes_mine", [](double x) { return my_math::cdf(x); }, inputs, 1'000'000);
}