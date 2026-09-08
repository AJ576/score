/*
 * src/bindings/nanobindings.cpp
 *
 * nanobind wrapping for the Score library.  Compiled into the
 * _score_native extension module that lives next to the Python package
 * (python/score/).  Once the C++ stubs are filled in, every method here
 * automatically gets the real behavior.
 */

#include <score/core/exceptions.hpp>
#include <score/core/types.hpp>
#include <score/metrics/stock_metrics.hpp>
#include <score/statistics/cdf.hpp>
#include <score/statistics/descriptive.hpp>

#include <nanobind/make_iterator.h>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(_score_native, m) {
    m.doc() = "Score: native C/C++ extensions (statistics + stock metrics)";

    // ----------------------------------------------------------------------
    // Exception translation
    // ----------------------------------------------------------------------
    nb::exception<score::EmptySeriesError>(m, "EmptySeriesError");
    nb::exception<score::DimensionMismatchError>(m, "DimensionMismatchError");
    nb::exception<score::DomainError>(m, "DomainError");
    nb::exception<score::NotImplementedError>(m, "NativeNotImplementedError");

    // ----------------------------------------------------------------------
    // score::Series<double>
    // ----------------------------------------------------------------------
    nb::class_<score::Series<double>>(m, "Series")
        .def(nb::init<>(), "Construct an empty series.")
        .def(nb::init<std::vector<double>>(), "data"_a, "Construct from a sequence of doubles.")
        .def(nb::init<std::vector<double>, std::vector<std::string>>(), "data"_a, "dates"_a,
             "Construct from values and matching ISO date strings.")
        .def("__len__", &score::Series<double>::size)
        .def("__getitem__",
             [](const score::Series<double>& s, std::size_t idx) {
                 if (idx >= s.size()) {
                     throw nb::index_error();
                 }
                 return s[idx];
             })
        .def(
            "__iter__",
            [](const score::Series<double>& s) {
                return nb::make_iterator(nb::type<score::Series<double>>(), "iterator", s.begin(),
                                         s.end());
            },
            nb::keep_alive<0, 1>())
        .def("size", &score::Series<double>::size)
        .def("empty", &score::Series<double>::empty)
        .def("values", &score::Series<double>::values, nb::rv_policy::reference_internal)
        .def("dates", &score::Series<double>::dates, nb::rv_policy::reference_internal)
        .def("__repr__", [](const score::Series<double>& s) {
            return "<score.Series size=" + std::to_string(s.size()) + ">";
        });

    // ----------------------------------------------------------------------
    // score::statistics::DescriptiveStats
    // ----------------------------------------------------------------------
    nb::class_<score::statistics::DescriptiveStats>(m, "DescriptiveStats")
        .def(nb::init<const score::Series<double>&>(), "series"_a, nb::keep_alive<1, 2>(),
             "Construct from a Series.  The Series must outlive this object.")
        .def("mean", &score::statistics::DescriptiveStats::mean)
        .def("variance", &score::statistics::DescriptiveStats::variance, "sample"_a = true)
        .def("stddev", &score::statistics::DescriptiveStats::stddev, "sample"_a = true)
        .def("median", &score::statistics::DescriptiveStats::median)
        .def("min", &score::statistics::DescriptiveStats::min)
        .def("max", &score::statistics::DescriptiveStats::max)
        .def("range", &score::statistics::DescriptiveStats::range)
        .def("quantile", &score::statistics::DescriptiveStats::quantile, "q"_a)
        .def("skewness", &score::statistics::DescriptiveStats::skewness)
        .def("kurtosis", &score::statistics::DescriptiveStats::kurtosis);

    // ----------------------------------------------------------------------
    // score::metrics::StockAnalyzer
    // ----------------------------------------------------------------------
    nb::class_<score::metrics::StockAnalyzer>(m, "StockAnalyzer")
        .def(nb::init<const score::Series<double>&>(), "prices"_a, nb::keep_alive<1, 2>(),
             "Construct from a price Series.  The Series must outlive this object.")
        .def("simple_returns", &score::metrics::StockAnalyzer::simple_returns)
        .def("log_returns", &score::metrics::StockAnalyzer::log_returns)
        .def("cumulative_return", &score::metrics::StockAnalyzer::cumulative_return)
        .def("annualized_return", &score::metrics::StockAnalyzer::annualized_return,
             "periods_per_year"_a = 252)
        .def("annualized_volatility", &score::metrics::StockAnalyzer::annualized_volatility,
             "periods_per_year"_a = 252)
        .def("sharpe_ratio", &score::metrics::StockAnalyzer::sharpe_ratio, "risk_free_rate"_a = 0.0,
             "periods_per_year"_a = 252)
        .def("max_drawdown", &score::metrics::StockAnalyzer::max_drawdown);

    // ----------------------------------------------------------------------
    // score::statistics::cdf (normal distribution CDF)
    // ----------------------------------------------------------------------
    m.def("normal_cdf", &my_math::cdf, "x"_a, "mean"_a = 0.0, "stddev"_a = 1.0,
          "Standard/normal CDF via a rational erfc approximation.");

    m.def(
        "normal_cdf_batch",
        [](nb::ndarray<const double, nb::ndim<1>, nb::c_contig> x, double mean, double stddev) {
            const size_t n = x.shape(0);
            auto* out = new double[n];
            for (size_t i = 0; i < n; ++i) {
                out[i] = my_math::cdf(x(i), mean, stddev);
            }
            nb::capsule owner(out, [](void* p) noexcept { delete[] static_cast<double*>(p); });
            return nb::ndarray<nb::numpy, double, nb::ndim<1>>(out, {n}, owner);
        },
        "x"_a, "mean"_a = 0.0, "stddev"_a = 1.0,
        "Vectorized normal_cdf over a 1-D array of x values.");

    // ----------------------------------------------------------------------
    // Module metadata
    // ----------------------------------------------------------------------
    m.attr("__version__") = "0.1.0";
}
