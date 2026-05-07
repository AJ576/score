/*
 * src/c_api.cpp
 *
 * Implementation of the public C API.  This file is FULLY WIRED — it
 * forwards each C call into the corresponding C++ method, translating
 * exceptions into ScoreStatus codes.  When the user fills in the C++
 * methods, the C API automatically picks up the new behavior.
 */

#include <score/c_api.h>

#include <score/core/exceptions.hpp>
#include <score/core/types.hpp>
#include <score/metrics/stock_metrics.hpp>
#include <score/statistics/descriptive.hpp>

#include <new>
#include <vector>

// ---------------------------------------------------------------------------
// Opaque struct definitions.
//
// A ScoreSeries owns its vector<double>.  The C++ value-type classes
// (DescriptiveStats, StockAnalyzer) hold a reference to the series, so the
// C wrappers also hold a (non-owning) pointer back to the series so we
// can keep the reference alive.
// ---------------------------------------------------------------------------
struct ScoreSeries {
    score::Series<double> impl;
};

struct ScoreDescriptiveStats {
    score::statistics::DescriptiveStats impl;
    explicit ScoreDescriptiveStats(const score::Series<double>& s) : impl(s) {}
};

struct ScoreStockAnalyzer {
    score::metrics::StockAnalyzer impl;
    explicit ScoreStockAnalyzer(const score::Series<double>& s) : impl(s) {}
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
namespace {

template <typename Fn>
ScoreStatus guarded(double* out, Fn&& fn) noexcept {
    if (out == nullptr) {
        return SCORE_E_INVALID_ARG;
    }
    try {
        *out = fn();
        return SCORE_OK;
    } catch (const score::EmptySeriesError&) {
        return SCORE_E_EMPTY_SERIES;
    } catch (const score::DomainError&) {
        return SCORE_E_DOMAIN;
    } catch (const score::NotImplementedError&) {
        return SCORE_E_NOT_IMPL;
    } catch (const std::exception&) {
        return SCORE_E_UNKNOWN;
    } catch (...) {
        return SCORE_E_UNKNOWN;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Status
// ---------------------------------------------------------------------------
extern "C" const char* score_status_str(ScoreStatus status) {
    switch (status) {
        case SCORE_OK:             return "ok";
        case SCORE_E_INVALID_ARG:  return "invalid argument";
        case SCORE_E_EMPTY_SERIES: return "empty series";
        case SCORE_E_DOMAIN:       return "domain error";
        case SCORE_E_NOT_IMPL:     return "not implemented";
        case SCORE_E_UNKNOWN:      return "unknown error";
    }
    return "unknown status code";
}

// ---------------------------------------------------------------------------
// Series
// ---------------------------------------------------------------------------
extern "C" ScoreSeries* score_series_create(const double* data, size_t len) {
    if (data == nullptr && len > 0) {
        return nullptr;
    }
    try {
        std::vector<double> v(data, data + len);
        return new (std::nothrow) ScoreSeries{score::Series<double>(std::move(v))};
    } catch (...) {
        return nullptr;
    }
}

extern "C" void score_series_destroy(ScoreSeries* s) {
    delete s;
}

extern "C" size_t score_series_size(const ScoreSeries* s) {
    return s == nullptr ? 0u : s->impl.size();
}

// ---------------------------------------------------------------------------
// DescriptiveStats
// ---------------------------------------------------------------------------
extern "C" ScoreDescriptiveStats* score_stats_create(const ScoreSeries* s) {
    if (s == nullptr) {
        return nullptr;
    }
    return new (std::nothrow) ScoreDescriptiveStats(s->impl);
}

extern "C" void score_stats_destroy(ScoreDescriptiveStats* st) {
    delete st;
}

extern "C" ScoreStatus score_stats_mean(const ScoreDescriptiveStats* st, double* out) {
    if (st == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return st->impl.mean(); });
}

extern "C" ScoreStatus
score_stats_variance(const ScoreDescriptiveStats* st, int sample, double* out) {
    if (st == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return st->impl.variance(sample != 0); });
}

extern "C" ScoreStatus
score_stats_stddev(const ScoreDescriptiveStats* st, int sample, double* out) {
    if (st == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return st->impl.stddev(sample != 0); });
}

extern "C" ScoreStatus score_stats_min(const ScoreDescriptiveStats* st, double* out) {
    if (st == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return st->impl.min(); });
}

extern "C" ScoreStatus score_stats_max(const ScoreDescriptiveStats* st, double* out) {
    if (st == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return st->impl.max(); });
}

extern "C" ScoreStatus
score_stats_quantile(const ScoreDescriptiveStats* st, double q, double* out) {
    if (st == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return st->impl.quantile(q); });
}

// ---------------------------------------------------------------------------
// StockAnalyzer
// ---------------------------------------------------------------------------
extern "C" ScoreStockAnalyzer* score_stock_create(const ScoreSeries* prices) {
    if (prices == nullptr) {
        return nullptr;
    }
    return new (std::nothrow) ScoreStockAnalyzer(prices->impl);
}

extern "C" void score_stock_destroy(ScoreStockAnalyzer* sa) {
    delete sa;
}

extern "C" ScoreStatus
score_stock_cumulative_return(const ScoreStockAnalyzer* sa, double* out) {
    if (sa == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return sa->impl.cumulative_return(); });
}

extern "C" ScoreStatus
score_stock_annualized_return(const ScoreStockAnalyzer* sa, int periods, double* out) {
    if (sa == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return sa->impl.annualized_return(periods); });
}

extern "C" ScoreStatus
score_stock_annualized_volatility(const ScoreStockAnalyzer* sa, int periods, double* out) {
    if (sa == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return sa->impl.annualized_volatility(periods); });
}

extern "C" ScoreStatus
score_stock_sharpe_ratio(const ScoreStockAnalyzer* sa, double rf, int periods, double* out) {
    if (sa == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return sa->impl.sharpe_ratio(rf, periods); });
}

extern "C" ScoreStatus score_stock_max_drawdown(const ScoreStockAnalyzer* sa, double* out) {
    if (sa == nullptr) return SCORE_E_INVALID_ARG;
    return guarded(out, [&] { return sa->impl.max_drawdown(); });
}
