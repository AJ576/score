#include "cdf.hpp"
#include<iostream>
#include<functional>
#include<cmath>

// This is a basic implementation just to test the baseline runtime.
double normal_pdf(double x, double mean = 0.0, double stddev = 1.0);
namespace
{
    double simpson_rule(const std::function<double(double)>& f, double a, double b, int n)
    {
        if(n%2 != 0) n++;

        double h = (b-a)/n;
        double sum = f(a)+f(b);
        double mul;
        for(int i=1;i<n;i++)
        {
            if (i%2 == 0)
            {
                mul = 2.0;
            }
            else
            {
                mul = 4.0;
            }
            double x = a+i*h;
            sum+=mul*f(x);
        }
        return (h/3.0)*sum;
    }
}


//calculating the cdf using simpsons rule
namespace naive
{
    double cdf(double x, double mean, double stddev, int n)
    {

        constexpr double kTailStdDevs = 8.3; // beyond this, tail mass < double's epsilon
        double lower_bound = mean - kTailStdDevs * stddev;

        if (x <= lower_bound)
            return 0.0;
        double upper_bound = mean + kTailStdDevs * stddev;
        if(x >= upper_bound)
            return 1.0;

        auto pdf = [mean, stddev](double a){return normal_pdf(a,mean,stddev);};
        return simpson_rule(pdf, lower_bound, x, n );
    }
}
