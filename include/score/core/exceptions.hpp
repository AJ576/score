#pragma once

#include <stdexcept>
#include <string>

namespace score {

/// Base class for all Score-specific exceptions.  Inherits from
/// std::runtime_error so callers can catch by std::exception too.
class ScoreException : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class EmptySeriesError : public ScoreException {
  public:
    EmptySeriesError() : ScoreException("Score: operation requires a non-empty series") {}
    explicit EmptySeriesError(const std::string& what) : ScoreException(what) {}
};

class DimensionMismatchError : public ScoreException {
  public:
    explicit DimensionMismatchError(const std::string& what) : ScoreException(what) {}
};

class DomainError : public ScoreException {
  public:
    explicit DomainError(const std::string& what) : ScoreException(what) {}
};

class NotImplementedError : public ScoreException {
  public:
    explicit NotImplementedError(const std::string& what)
        : ScoreException("Not implemented: " + what) {}
};

} // namespace score
