#!/usr/bin/env bash
# Configures, builds and runs the unit tests (intended to run in the container).
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BUILD_DIR="build/noble"

cmake -S . -B "${BUILD_DIR}" -G Ninja \
	-DCMAKE_BUILD_TYPE=Debug \
	-DBUILD_TESTS=ON \
	-DENABLE_CLANG_TIDY=OFF \
	-DENABLE_CLANG_FORMAT_CHECK=OFF

cmake --build "${BUILD_DIR}" --parallel

ctest --test-dir "${BUILD_DIR}" --output-on-failure
