/*
 * src/c_kernels/kernels.c
 *
 * Pure C17 numerical primitives.  Compiled with -std=c17, no extensions.
 *
 * Each function is intentionally a stub — the user fills in the loop
 * body.  Hints are given as comments.  Kahan / Neumaier compensated
 * summation is a worthwhile upgrade once the naive version is working.
 */

#include <score/c_kernels/kernels.h>

#include <math.h>

double score_k_sum(const double* xs, size_t len) {
    /* TODO: naive Σ x_i loop, then consider compensated summation. */
    (void)xs;
    (void)len;
    return 0.0;
}

double score_k_dot(const double* xs, const double* ys, size_t len) {
    /* TODO: Σ (x_i * y_i) */
    (void)xs;
    (void)ys;
    (void)len;
    return 0.0;
}

double score_k_min(const double* xs, size_t len) {
    /* TODO: scan for the smallest element.
     * Return +INFINITY when len == 0. */
    if (len == 0) {
        return INFINITY;
    }
    (void)xs;
    return 0.0;
}

double score_k_max(const double* xs, size_t len) {
    /* TODO: scan for the largest element.
     * Return -INFINITY when len == 0. */
    if (len == 0) {
        return -INFINITY;
    }
    (void)xs;
    return 0.0;
}

double score_k_accumulate_squared_dev(const double* xs, size_t len, double mean) {
    /* TODO: Σ (x_i - mean)² */
    (void)xs;
    (void)len;
    (void)mean;
    return 0.0;
}
