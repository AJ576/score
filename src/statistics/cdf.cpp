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
            if (i%2 == 0) {
                mul = 2.0;
            } else {
                mul = 4.0;
            }
            double x = a+i*h;
            sum+=mul*f(x);
        }
        return (h/3.0)*sum;
    }

    double polevl(double x, const double* coeff, int N)
    {
        double ans = coeff[0];
        for(int i = 1; i <= N; i++)
        {
            ans = ans*x+coeff[i];
        }
        return ans;
    }

    double p1evl(double x, const double* coeff, int N)
    {
        double ans = x + coeff[0];
        for(int i = 1; i <= N-1; i++)
        {
            ans = ans*x+coeff[i];
        }
        return ans;
    }

    //values were stolen from cephes library for ndtr

    // ---------------------------------------------------------------------
    // Coefficients for erf(x), |x| <= 1:
    //     erf(x) = x * T(x^2) / U(x^2)
    // T is degree 4 (5 stored coefficients, non-monic -> polevl).
    // U is degree 5, monic (5 stored coefficients, leading 1 implicit -> p1evl).
    // ---------------------------------------------------------------------
    
    const double T[5] = {
        9.60497373987051638749E0,
        9.00260197203842689217E1,
        2.23200534594684319226E3,
        7.00332514112805075473E3,
        5.55923013010394962768E4
    };
    
    const double U[5] = {
        3.35617141647503099647E1,
        5.21357949780152679795E2,
        4.59432382970980127987E3,
        2.26290000613890934246E4,
        4.92673942608635921086E4
    };
    
    // ---------------------------------------------------------------------
    // Coefficients for erfc(x)*exp(x^2), 1 <= x < 8:
    //     erfc(x) = exp(-x^2) * P(x) / Q(x)
    // P is degree 8 (9 stored coefficients, non-monic -> polevl).
    // Q is degree 8, monic (8 stored coefficients -> p1evl).
    // ---------------------------------------------------------------------
    
    const double P[9] = {
        2.46196981473530512524E-10,
        5.64189564831068821977E-1,
        7.46321056442269912687E0,
        4.86371970985681366614E1,
        1.96520832956077098242E2,
        5.26445194995477358631E2,
        9.34528527171957607540E2,
        1.02755188689515710272E3,
        5.57535335369399327526E2
    };
    
    const double Q[8] = {
        1.32281951154744992508E1,
        8.67072140885989742329E1,
        3.54937778887819891062E2,
        9.75708501743205489753E2,
        1.82390916687909736289E3,
        2.24633760818710981792E3,
        1.65666309194161350182E3,
        5.57535340817727675546E2
    };
    
    // ---------------------------------------------------------------------
    // Coefficients for erfc(x)*exp(x^2), x >= 8:
    //     erfc(x) = exp(-x^2) * R(x) / S(x)
    // R is degree 5 (6 stored coefficients, non-monic -> polevl).
    // S is degree 6, monic (6 stored coefficients -> p1evl).
    // ---------------------------------------------------------------------
    
    const double R[6] = {
        5.64189583547755073984E-1,
        1.27536670759978104416E0,
        5.01905042251180477414E0,
        6.16021097993053585195E0,
        7.40974269950448939160E0,
        2.97886665372100240670E0
    };
    
    const double S[6] = {
        2.26052863220117276590E0,
        9.39603524938001434673E0,
        1.20489539808096656605E1,
        1.70814450747565897222E1,
        9.60896809063285878198E0,
        3.36907645100081516050E0
    };

    //not this one tho
    constexpr double MAXLOG = 7.09782712893383996843E2; // ln(DBL_MAX), underflow guard
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

namespace std_math
{
    double cdf_erfc(double x, double mean, double stddev)
    {
        double z = (x-mean)/(stddev * std::sqrt(2.0));
        double cdf = 0.5*std::erfc(-z);
        return cdf;
    }

    double cdf_erf(double x, double mean, double stddev)
    {
        double z = (x-mean)/(stddev * std::sqrt(2.0));
        double cdf = 0.5*(1+std::erf(z));
        return cdf;
    }
} // namespace std_math

namespace my_math
{
    double my_erf(double x)
    {
        if (std::fabs(x) > 1.0) {
        return 1.0 - my_erfc(x);
        }
        double z = x*x;
        return x*polevl(z, T, 4)/p1evl(z, U, 5);
    }

    double my_erfc(double a)
    {
        double x = std::fabs(a);
        if (x < 1.0) {
            return 1.0 - my_erf(a);
        }
    
        double z = -a * a;
        if (z < -MAXLOG) {
            // underflow: result is indistinguishable from the limit
            return (a < 0) ? 2.0 : 0.0;
        }
    
        // NOTE: plain exp() here, not Cephes's precision-guarded expx2().
        // This is a deliberate simplification, we lose some precision with x*x and then exp-in it
        //but i am not sure we need that much precision tbh. 
        double exp_z = std::exp(z);
    
        double p, q;
        if (x < 8.0) {
            p = polevl(x, P, 8);
            q = p1evl(x, Q, 8);
        } else {
            p = polevl(x, R, 5);
            q = p1evl(x, S, 6);
        }
    
        double y = (exp_z * p) / q;
        if (a < 0) {
            y = 2.0 - y;
        }
        return y;
        }
    double cdf(double x, double mean, double stddev)
    {
        const double z = (x - mean) / (stddev * std::sqrt(2.0));
        return 0.5 * my_erfc(-z);
    }
}
