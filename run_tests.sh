#!/usr/bin/env bash
# Configures, builds and runs tests; sanitizer runs disable coverage instrumentation.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

integration=OFF
sanitizers=OFF
coverage=ON
for argument in "$@"; do
	case "${argument}" in
		INTEGRATION)
			integration=ON
			;;
		SANITIZER)
			sanitizers=ON
			integration=ON
			coverage=OFF
			;;
		*)
			echo "Unknown argument: ${argument}" >&2
			echo "Usage: $0 [INTEGRATION] [SANITIZER]" >&2
			exit 2
			;;
	esac
done

BUILD_DIR="build/noble"

cmake -S . -B "${BUILD_DIR}" -G Ninja \
	-DCMAKE_BUILD_TYPE=Debug \
	-DBUILD_TESTS=ON \
	-DENABLE_CLANG_TIDY=ON \
	-DENABLE_CLANG_FORMAT_CHECK=OFF \
	-DENABLE_COVERAGE="${coverage}" \
	-DENABLE_SANITIZERS="${sanitizers}" \
	-DENABLE_INTEGRATION_TESTS="${integration}"

cmake --build "${BUILD_DIR}" --parallel

ctest --test-dir "${BUILD_DIR}" --output-on-failure

if [[ "${coverage}" == "ON" ]]; then
	gcovr --root . \
		--object-directory "${BUILD_DIR}" \
		--filter 'src/' \
		--exclude 'src/main.cpp' \
		--print-summary \
		--txt
fi
