#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace score {

/// A 1-D numeric series with optional date labels.
///
/// Series owns its data and is the canonical container shared between the
/// statistics and metrics modules.  It deliberately exposes both
/// `operator[]` (no bounds check) and `at()` (bounds-checked) so that the
/// hot numerical paths can use the unchecked variant via `data()`.
template <typename T = double>
class Series {
  public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    Series() = default;

    explicit Series(std::vector<T> data) : data_(std::move(data)) {}

    Series(std::vector<T> data, std::vector<std::string> dates)
        : data_(std::move(data)), dates_(std::move(dates)) {
        if (!dates_.empty() && dates_.size() != data_.size()) {
            throw std::invalid_argument("Series: dates size must match data size or be empty");
        }
    }

    [[nodiscard]] size_type size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

    T& operator[](size_type idx) { return data_[idx]; }
    const T& operator[](size_type idx) const { return data_[idx]; }

    T& at(size_type idx) { return data_.at(idx); }
    const T& at(size_type idx) const { return data_.at(idx); }

    T* data() noexcept { return data_.data(); }
    const T* data() const noexcept { return data_.data(); }

    iterator begin() noexcept { return data_.begin(); }
    iterator end() noexcept { return data_.end(); }
    const_iterator begin() const noexcept { return data_.begin(); }
    const_iterator end() const noexcept { return data_.end(); }
    const_iterator cbegin() const noexcept { return data_.cbegin(); }
    const_iterator cend() const noexcept { return data_.cend(); }

    const std::vector<T>& values() const noexcept { return data_; }
    const std::vector<std::string>& dates() const noexcept { return dates_; }

  private:
    std::vector<T> data_;
    std::vector<std::string> dates_;
};

using SeriesD = Series<double>;

} // namespace score
