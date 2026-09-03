#pragma once
namespace naive
{
    double cdf(double x,
               double mean = 0.0,
               double stddev = 1.0,
               int n = 100000);
}