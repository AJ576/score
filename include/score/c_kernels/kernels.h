/*
 * score/c_kernels/kernels.h
 *
 * Pure C17 numerical primitives.  These are the lowest layer of the Score
 * library; the C++ classes in score::statistics and score::metrics call into
 * them.  Keeping them in C makes the C/C++ split visible and exercises the
 * C17 standard for real (-std=c17, no compiler extensions).
 *
 * All functions are header-declared with C linkage so they can be called
 * from C, C++, and any FFI-capable language.
 */

#ifndef SCORE_C_KERNELS_KERNELS_H
#define SCORE_C_KERNELS_KERNELS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Σ x_i over the first `len` elements of xs.
 * Returns 0.0 when len == 0. */
double score_k_sum(const double* xs, size_t len);

/* Σ (x_i * y_i).  xs and ys must each have at least `len` elements.
 * Returns 0.0 when len == 0. */
double score_k_dot(const double* xs, const double* ys, size_t len);

/* min{x_i} over xs[0..len).  Returns +INFINITY when len == 0. */
double score_k_min(const double* xs, size_t len);

/* max{x_i} over xs[0..len).  Returns -INFINITY when len == 0. */
double score_k_max(const double* xs, size_t len);

/* Σ (x_i - mean)².  Useful for variance/stddev computations.
 * Returns 0.0 when len == 0. */
double score_k_accumulate_squared_dev(const double* xs, size_t len, double mean);

#ifdef __cplusplus
}
#endif

#endif /* SCORE_C_KERNELS_KERNELS_H */
