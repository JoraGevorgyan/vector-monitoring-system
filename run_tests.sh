#!/usr/bin/env bash
# Configures, builds and runs the unit tests with coverage instrumentation,
# then prints the coverage statistics (intended to run in the container).
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BUILD_DIR="build/noble"

cmake -S . -B "${BUILD_DIR}" -G Ninja \
	-DCMAKE_BUILD_TYPE=Debug \
	-DBUILD_TESTS=ON \
	-DENABLE_CLANG_TIDY=ON \
	-DENABLE_CLANG_FORMAT_CHECK=OFF \
	-DENABLE_COVERAGE=ON

cmake --build "${BUILD_DIR}" --parallel

ctest --test-dir "${BUILD_DIR}" --output-on-failure

gcovr --root . \
	--object-directory "${BUILD_DIR}" \
	--filter 'src/' \
	--exclude 'src/main.cpp' \
	--print-summary \
	--txt
