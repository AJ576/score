#include <chrono>
#include <iostream>
#include <vector>
#include <iomanip>
#include "../statistics/cdf.hpp"

int main()
{
    constexpr int warmup_iterations = 1'000;
    constexpr int iterations = 10'000; // big enough to swamp clock resolution

    const std::vector<double> inputs = {
        -10, -5, -2, -1, 0, 1, 2, 5, 10
    };

    for (double x : inputs)
    {
        volatile double sink = 0.0; // stops the compiler discarding the loop

        // warm-up: pay cache/branch-predictor cold-start cost before timing
        for (int i = 0; i < warmup_iterations; ++i)
            sink = naive::cdf(x + i * 1e-15);

        auto start = std::chrono::steady_clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            // perturb slightly so the compiler can't collapse N identical calls into 1
            sink = naive::cdf(x + i * 1e-15);//technincally overwrites but averaging out is just gonna lose precision so whatever. 
        }

        auto end = std::chrono::steady_clock::now();

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        const double time_per_call =
            static_cast<double>(elapsed.count()) / iterations;

        std::cout << std::setprecision(17);
        std::cout << "x = " << x
                  << " | CDF ~= " << sink
                  << " | time = " << time_per_call << " ns\n";
    }
}