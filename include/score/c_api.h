/*
 * score/c_api.h
 *
 * Public C API for the Score library.  Wraps the C++ classes in a flat,
 * extern "C" surface using opaque pointers (SQLite-style) so that:
 *   * pure C clients can use the library,
 *   * Python callers can use ctypes/CFFI as an alternative to pybind11,
 *   * non-C++ languages (Rust, Go, etc.) can FFI in cleanly.
 *
 * Methods that can fail return a ScoreStatus and write the result through
 * an out-pointer.  Methods that cannot fail (size queries, min/max on a
 * known non-empty series, etc.) return their value directly.
 */

#ifndef SCORE_C_API_H
#define SCORE_C_API_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------- */
/* Opaque types                                                            */
/* ----------------------------------------------------------------------- */
typedef struct ScoreSeries ScoreSeries;
typedef struct ScoreDescriptiveStats ScoreDescriptiveStats;
typedef struct ScoreStockAnalyzer ScoreStockAnalyzer;

/* ----------------------------------------------------------------------- */
/* Status codes                                                            */
/* ----------------------------------------------------------------------- */
typedef enum {
    SCORE_OK = 0,
    SCORE_E_INVALID_ARG = 1,
    SCORE_E_EMPTY_SERIES = 2,
    SCORE_E_DOMAIN = 3,
    SCORE_E_NOT_IMPL = 4,
    SCORE_E_UNKNOWN = 99
} ScoreStatus;

/* Human-readable string for a status code.  Returned pointer points at
 * static storage; do not free. */
const char* score_status_str(ScoreStatus status);

/* ----------------------------------------------------------------------- */
/* Series                                                                  */
/* ----------------------------------------------------------------------- */
ScoreSeries* score_series_create(const double* data, size_t len);
void score_series_destroy(ScoreSeries* s);
size_t score_series_size(const ScoreSeries* s);

/* ----------------------------------------------------------------------- */
/* DescriptiveStats                                                        */
/* ----------------------------------------------------------------------- */
ScoreDescriptiveStats* score_stats_create(const ScoreSeries* s);
void score_stats_destroy(ScoreDescriptiveStats* st);

ScoreStatus score_stats_mean(const ScoreDescriptiveStats* st, double* out);
ScoreStatus score_stats_variance(const ScoreDescriptiveStats* st, int sample, double* out);
ScoreStatus score_stats_stddev(const ScoreDescriptiveStats* st, int sample, double* out);
ScoreStatus score_stats_min(const ScoreDescriptiveStats* st, double* out);
ScoreStatus score_stats_max(const ScoreDescriptiveStats* st, double* out);
ScoreStatus score_stats_quantile(const ScoreDescriptiveStats* st, double q, double* out);

/* ----------------------------------------------------------------------- */
/* StockAnalyzer                                                           */
/* ----------------------------------------------------------------------- */
ScoreStockAnalyzer* score_stock_create(const ScoreSeries* prices);
void score_stock_destroy(ScoreStockAnalyzer* sa);

ScoreStatus score_stock_cumulative_return(const ScoreStockAnalyzer* sa, double* out);
ScoreStatus score_stock_annualized_return(const ScoreStockAnalyzer* sa, int periods, double* out);
ScoreStatus score_stock_annualized_volatility(const ScoreStockAnalyzer* sa, int periods,
                                              double* out);
ScoreStatus score_stock_sharpe_ratio(const ScoreStockAnalyzer* sa, double rf, int periods,
                                     double* out);
ScoreStatus score_stock_max_drawdown(const ScoreStockAnalyzer* sa, double* out);

#ifdef __cplusplus
}
#endif

#endif /* SCORE_C_API_H */
