#include <score/statistics/wiener_process.hpp>

#include <cmath>
#include <random>

namespace wiener {
std::vector<double> wiener_process(double total_time, double time_step,
                                   std::optional<unsigned int> seed) {
    // set seed- TODO: MAKE a seperate function for seeding shit and generating a seed
    unsigned int actualSeed;
    if (seed.has_value()) {
        // User provided a seed -> Use it for reproducibility
        actualSeed = seed.value();
    } else {
        // User left it blank -> Generate a truly random seed
        std::random_device rd;
        actualSeed = rd();
    }
    std::mt19937 gen(actualSeed);
    std::normal_distribution<double> Z; // gets a normal distrbution

    std::vector<double> W;
    W.push_back(0.0);
    double t = 0.0;
    while (t + time_step <= total_time) {
        t += time_step; // advance FIRST, so t now means "the timestamp we're about to compute"
        const double W_old = W.back();
        const double W_new = W_old + Z(gen) * std::sqrt(time_step);
        W.push_back(W_new);
        // correctly labels W_new: W.back() really is W(t)
    }

    return W;
}
} // namespace wiener