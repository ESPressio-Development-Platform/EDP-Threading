#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="$(cd "${ROOT}/.." && pwd)"
CXX_BIN="${CXX:-c++}"
SOURCE="${ROOT}/tests/FailClosedOrdinaryMutexTests.cpp"
OUTPUT="${ROOT}/tests/.fail-closed-ordinary-mutex-tests"

cleanup() {
    rm -f "${OUTPUT}"
}
trap cleanup EXIT

compile_tests() {
    "${CXX_BIN}" \
        -std=c++20 \
        -Wall \
        -Wextra \
        -Wpedantic \
        -Werror \
        -pthread \
        "$@" \
        -I"${ROOT}/src" \
        -I"${WORKSPACE}/EDP-System/src" \
        -I"${WORKSPACE}/EDP-Platform/src" \
        -I"${WORKSPACE}/EDP-Clock/src" \
        -I"${WORKSPACE}/EDP-BoundedTopology/src" \
        -I"${WORKSPACE}/EDP-Memory/src" \
        "${SOURCE}" -o "${OUTPUT}"
}

SANITIZER_MODE="${EDP_THREADING_SANITIZER_MODE:-}"
if [[ "${EDP_THREADING_SANITIZERS:-0}" == "1" && -z "${SANITIZER_MODE}" ]]; then
    SANITIZER_MODE="address,undefined"
fi

case "${SANITIZER_MODE}" in
    "") compile_tests ;;
    address) compile_tests -g -fno-omit-frame-pointer -fsanitize=address ;;
    undefined) compile_tests -g -fno-omit-frame-pointer -fsanitize=undefined ;;
    address,undefined) compile_tests -g -fno-omit-frame-pointer -fsanitize=address,undefined ;;
    *)
        echo "Unsupported EDP_THREADING_SANITIZER_MODE: ${SANITIZER_MODE}" >&2
        exit 2
        ;;
esac

if [[ -n "${SANITIZER_MODE}" ]]; then
    ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0:abort_on_error=1}" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}" \
        "${OUTPUT}"
else
    "${OUTPUT}"
fi

echo "EDP-Threading fail-closed ordinary mutex tests: PASS"
