#!/usr/bin/env bash
# Convenience build script.  `make build` is the preferred entry point;
# this exists for users who want to invoke cmake explicitly.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build"

cd "${ROOT}"

echo "==> initializing submodules"
git submodule update --init --recursive

echo "==> configuring cmake (build dir: ${BUILD_DIR})"
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release

echo "==> building"
cmake --build "${BUILD_DIR}" -j"$(sysctl -n hw.ncpu 2>/dev/null || nproc)"

echo "==> done"
echo "Run tests with:  make test-cpp"
echo "Install Python:  make dev"
