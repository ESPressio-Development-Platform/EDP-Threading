#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="$(cd "${ROOT}/.." && pwd)"

required=(
    "EDP-System"
    "EDP-Platform"
    "EDP-Clock"
)

for dependency in "${required[@]}"; do
    if [[ ! -d "${WORKSPACE}/${dependency}/src" ]]; then
        echo "Missing sibling repository: ${WORKSPACE}/${dependency}" >&2
        echo "Clone EDP-System, EDP-Platform and EDP-Clock beside EDP-Threading." >&2
        exit 2
    fi
done

OUTPUT="${ROOT}/tests/.threading-foundation-tests"

SANITIZER_FLAGS=()
if [[ "${EDP_THREADING_SANITIZERS:-1}" != "0" ]]; then
    SANITIZER_FLAGS=(
        -g
        -fno-omit-frame-pointer
        -fsanitize=address,undefined
    )
fi

cleanup() {
    rm -f "${OUTPUT}"
}
trap cleanup EXIT

c++ \
    -std=c++17 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Werror \
    "${SANITIZER_FLAGS[@]}" \
    -I"${ROOT}/src" \
    -I"${WORKSPACE}/EDP-System/src" \
    -I"${WORKSPACE}/EDP-Platform/src" \
    -I"${WORKSPACE}/EDP-Clock/src" \
    "${ROOT}/tests/ThreadingFoundationTests.cpp" \
    -o "${OUTPUT}"

if [[ "${EDP_THREADING_SANITIZERS:-1}" != "0" ]]; then
    ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0:abort_on_error=1}" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}" \
        "${OUTPUT}"
else
    "${OUTPUT}"
fi

echo "EDP-Threading host foundation tests: PASS"
