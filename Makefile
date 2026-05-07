# Convenience wrappers around CMake / pip / pytest.  All real build logic
# lives in CMakeLists.txt, pyproject.toml, and the scripts under scripts/.

BUILD_DIR ?= build
JOBS      ?= $(shell sysctl -n hw.ncpu 2>/dev/null || nproc)
PYTHON    ?= python3
PIP       ?= $(PYTHON) -m pip

.PHONY: help build configure clean rebuild submodules dev test test-cpp test-py mock-data format

help:
	@echo "Score build targets:"
	@echo "  make submodules   - git submodule update --init --recursive"
	@echo "  make build        - configure + build C/C++ library and pybind11 module"
	@echo "  make rebuild      - clean and build"
	@echo "  make dev          - pip install -e .[dev] (editable Python install)"
	@echo "  make test-cpp     - run C++ tests via ctest"
	@echo "  make test-py      - run Python tests via pytest"
	@echo "  make test         - run both test suites"
	@echo "  make mock-data    - regenerate data/mock_stocks.json"
	@echo "  make format       - clang-format C/C++ sources"
	@echo "  make clean        - remove build directory"

submodules:
	git submodule update --init --recursive

configure: submodules
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

build: configure
	cmake --build $(BUILD_DIR) -j$(JOBS)

rebuild: clean build

clean:
	rm -rf $(BUILD_DIR)

dev:
	$(PIP) install -e ".[dev]"

test-cpp: build
	cd $(BUILD_DIR) && ctest --output-on-failure

test-py:
	$(PYTHON) -m pytest tests/python -v

test: test-cpp test-py

mock-data:
	$(PYTHON) scripts/gen_mock_data.py

format:
	@command -v clang-format >/dev/null 2>&1 || { echo "clang-format not found"; exit 1; }
	find include src tests/cpp -type f \( -name '*.hpp' -o -name '*.cpp' -o -name '*.h' -o -name '*.c' \) \
		-print0 | xargs -0 clang-format -i
