/*
 * src/bindings/pybindings.cpp
 *
 * pybind11 wrapping for the Score library.  Compiled into the
 * _score_native extension module that lives next to the Python package
 * (python/score/).  Once the C++ stubs are filled in, every method here
 * automatically gets the real behavior.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <score/core/exceptions.hpp>
#include <score/core/types.hpp>
#include <score/metrics/stock_metrics.hpp>
#include <score/statistics/descriptive.hpp>

namespace py = pybind11;

PYBIND11_MODULE(_score_native, m) {
    m.doc() = "Score: native C/C++ extensions (statistics + stock metrics)";

    // ----------------------------------------------------------------------
    // Exception translation
    // ----------------------------------------------------------------------
    py::register_exception<score::EmptySeriesError>(m, "EmptySeriesError");
    py::register_exception<score::DimensionMismatchError>(m, "DimensionMismatchError");
    py::register_exception<score::DomainError>(m, "DomainError");
    py::register_exception<score::NotImplementedError>(m, "NativeNotImplementedError");

    // ----------------------------------------------------------------------
    // score::Series<double>
    // ----------------------------------------------------------------------
    py::class_<score::Series<double>>(m, "Series")
        .def(py::init<>(), "Construct an empty series.")
        .def(py::init<std::vector<double>>(),
             py::arg("data"),
             "Construct from a sequence of doubles.")
        .def(py::init<std::vector<double>, std::vector<std::string>>(),
             py::arg("data"), py::arg("dates"),
             "Construct from values and matching ISO date strings.")
        .def("__len__", &score::Series<double>::size)
        .def("__getitem__",
             [](const score::Series<double>& s, std::size_t idx) {
                 if (idx >= s.size()) {
                     throw py::index_error();
                 }
                 return s[idx];
             })
        .def("__iter__",
             [](const score::Series<double>& s) {
                 return py::make_iterator(s.begin(), s.end());
             },
             py::keep_alive<0, 1>())
        .def("size", &score::Series<double>::size)
        .def("empty", &score::Series<double>::empty)
        .def("values", &score::Series<double>::values,
             py::return_value_policy::reference_internal)
        .def("dates", &score::Series<double>::dates,
             py::return_value_policy::reference_internal)
        .def("__repr__",
             [](const score::Series<double>& s) {
                 return "<score.Series size=" + std::to_string(s.size()) + ">";
             });

    // ----------------------------------------------------------------------
    // score::statistics::DescriptiveStats
    // ----------------------------------------------------------------------
    py::class_<score::statistics::DescriptiveStats>(m, "DescriptiveStats")
        .def(py::init<const score::Series<double>&>(),
             py::arg("series"),
             py::keep_alive<1, 2>(),
             "Construct from a Series.  The Series must outlive this object.")
        .def("mean",     &score::statistics::DescriptiveStats::mean)
        .def("variance", &score::statistics::DescriptiveStats::variance,
             py::arg("sample") = true)
        .def("stddev",   &score::statistics::DescriptiveStats::stddev,
             py::arg("sample") = true)
        .def("median",   &score::statistics::DescriptiveStats::median)
        .def("min",      &score::statistics::DescriptiveStats::min)
        .def("max",      &score::statistics::DescriptiveStats::max)
        .def("range",    &score::statistics::DescriptiveStats::range)
        .def("quantile", &score::statistics::DescriptiveStats::quantile,
             py::arg("q"))
        .def("skewness", &score::statistics::DescriptiveStats::skewness)
        .def("kurtosis", &score::statistics::DescriptiveStats::kurtosis);

    // ----------------------------------------------------------------------
    // score::metrics::StockAnalyzer
    // ----------------------------------------------------------------------
    py::class_<score::metrics::StockAnalyzer>(m, "StockAnalyzer")
        .def(py::init<const score::Series<double>&>(),
             py::arg("prices"),
             py::keep_alive<1, 2>(),
             "Construct from a price Series.  The Series must outlive this "
             "object.")
        .def("simple_returns", &score::metrics::StockAnalyzer::simple_returns)
        .def("log_returns",    &score::metrics::StockAnalyzer::log_returns)
        .def("cumulative_return",
             &score::metrics::StockAnalyzer::cumulative_return)
        .def("annualized_return",
             &score::metrics::StockAnalyzer::annualized_return,
             py::arg("periods_per_year") = 252)
        .def("annualized_volatility",
             &score::metrics::StockAnalyzer::annualized_volatility,
             py::arg("periods_per_year") = 252)
        .def("sharpe_ratio",
             &score::metrics::StockAnalyzer::sharpe_ratio,
             py::arg("risk_free_rate") = 0.0,
             py::arg("periods_per_year") = 252)
        .def("max_drawdown", &score::metrics::StockAnalyzer::max_drawdown);

    // ----------------------------------------------------------------------
    // Module metadata
    // ----------------------------------------------------------------------
    m.attr("__version__") = "0.1.0";
}
