#include <score/core/types.hpp>

// Series<T> is a class template defined entirely in the header.  This
// translation unit exists so that score_core has at least one C++ source
// file in core/ even if the user later adds non-templated helpers here.

namespace score {

// Explicit instantiation of the most common specialization keeps debug
// info in one place and reduces compile time for downstream TUs.
template class Series<double>;

}  // namespace score
