# Convenience wrappers around CMake / pip / pytest.  All real build logic
# lives in CMakeLists.txt, pyproject.toml, and the scripts under scripts/.

BUILD_DIR ?= build
JOBS      ?= $(shell sysctl -n hw.ncpu 2>/dev/null || nproc)

UV ?= uv

# Prefer the project .venv (created by `make dev` / `uv venv`) so CMake finds
# pip-installed pybind11.
ifeq ($(wildcard .venv/bin/python),)
  PYTHON ?= python3
else
  PYTHON ?= .venv/bin/python
endif
VENV_PYTHON := $(CURDIR)/.venv/bin/python

.PHONY: help build configure clean rebuild submodules venv dev test test-cpp test-py mock-data format

help:
	@echo "Score build targets:"
	@echo "  make submodules   - git submodule update --init --recursive"
	@echo "  make venv         - create .venv with uv (no-op if it already exists)"
	@echo "  make dev          - uv pip install -e .[dev] (creates .venv if needed)"
	@echo "  make build        - clean, dev, configure, and build everything"
	@echo "  make rebuild      - alias for make build"
	@echo "  make test-cpp     - run C++ tests via ctest"
	@echo "  make test-py      - run Python tests via pytest"
	@echo "  make test         - run both test suites"
	@echo "  make mock-data    - regenerate data/mock_stocks.json"
	@echo "  make format       - clang-format C/C++ sources"
	@echo "  make clean        - remove build directory"

submodules:
	git submodule update --init --recursive

configure: clean submodules dev
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release \
		-DPython3_EXECUTABLE=$(VENV_PYTHON)

build: configure
	cmake --build $(BUILD_DIR) -j$(JOBS)

rebuild: build

clean:
	rm -rf $(BUILD_DIR)
	rm -f python/score/_score_native.*

venv:
	@test -d .venv || $(UV) venv

dev: venv
	$(UV) pip install -e ".[dev]"

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
