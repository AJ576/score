/**
 * @file kernels
 * @author your Dhabibhabi
 * @brief Pure C17 numerical primitives. Compiled with -std=c17, no extensions.
 * @version 0.1
 * @date 2026-05-30
 */
#include <score/c_kernels/kernels.h>

#include <math.h>

double score_k_sum(const double* xs, size_t len) {
    if (len == 0)
        return 0.0;
    double sum = 0;
    for (size_t i = 0; i < len; i++)
        sum += xs[i];
    return sum;
}

double score_k_dot(const double* xs, const double* ys, size_t len) {
    if (len == 0)
        return 0.0;
    double dot = 0;
    for (size_t i = 0; i < len; i++)
        dot += xs[i] * ys[i];
    return dot;
}

double score_k_min(const double* xs, size_t len) {
    if (len == 0)
        return INFINITY;
    double min = xs[0];
    for (size_t i = 1; i < len; i++)
        if (xs[i] < min)
            min = xs[i];
    return min;
}

double score_k_max(const double* xs, size_t len) {

    if (len == 0)
        return -INFINITY;
    double max = xs[0];
    for (size_t i = 1; i < len; i++)
        if (xs[i] > max)
            max = xs[i];
    return max;
}

double score_k_accumulate_squared_dev(const double* xs, size_t len, double mean) {
    if (len == 0)
        return 0.0;
    double sum = 0;
    for (size_t i = 0; i < len; i++)
        sum += pow(xs[i] - mean, 2);
    return sum;
}

double score_k_accumulate_cubed_dev(const double* xs, size_t len, double mean) {
    if (len == 0)
        return 0.0;
    double sum = 0;
    for (size_t i = 0; i < len; i++) {
        const double d = xs[i] - mean;
        sum += d * d * d;
    }
    return sum;
}

double score_k_accumulate_quartic_dev(const double* xs, size_t len, double mean) {
    if (len == 0)
        return 0.0;
    double sum = 0;
    for (size_t i = 0; i < len; i++) {
        const double d = xs[i] - mean;
        sum += d * d * d * d;
    }
    return sum;
}
