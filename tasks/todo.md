# C++ Kernel Migration

- [x] Remove the private C17 kernel layer.
- [x] Implement the numeric operations in the C++ statistics implementation.
- [x] Update build, tests, and documentation for the C++/nanobind architecture.
- [x] Configure and build the nanobind extension.
- [x] Configure, build, and run the focused C++ test suite.

## Review

- C++-only CMake configure and build succeeded.
- `score_tests "[stats]"` passed: 7 test cases, 8 assertions.
- Nanobind extension built successfully and `.venv/bin/python -c "import score"`
	reported version `0.1.0`.