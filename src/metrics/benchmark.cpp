#include <chrono>
#include <iostream>
#include <vector>
#include<iomanip>
#include "../statistics/cdf.hpp"

int main()
{
    constexpr int iterations = 1000;

    const std::vector<double> inputs = {
        -10, -5, -2, -1, 0, 1, 2, 5, 10
    };

    for (double x : inputs)
    {
        double result = 0.0;

        auto start = std::chrono::steady_clock::now();

        for (int i = 0; i < iterations; ++i)
        {
            result += naive::cdf(x);
        }

        auto end = std::chrono::steady_clock::now();

        const auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                end - start
            );

        const double average_result = result / iterations;
        const double time_per_call =
            static_cast<double>(elapsed.count()) / iterations;
        std::cout << std::setprecision(17);
        std::cout << "x = " << x
                  << " | CDF = " << average_result
                  << " | time = " << time_per_call << " ns\n";
    }
}