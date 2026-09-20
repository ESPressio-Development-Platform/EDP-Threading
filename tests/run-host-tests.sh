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

cleanup() {
    rm -f "${OUTPUT}"
}
trap cleanup EXIT

compile_tests() {
    c++ \
        -std=c++17 \
        -Wall \
        -Wextra \
        -Wpedantic \
        -Werror \
        "$@" \
        -I"${ROOT}/src" \
        -I"${WORKSPACE}/EDP-System/src" \
        -I"${WORKSPACE}/EDP-Platform/src" \
        -I"${WORKSPACE}/EDP-Clock/src" \
        "${ROOT}/tests/ThreadingFoundationTests.cpp" \
        -o "${OUTPUT}"
}

SANITIZER_MODE="${EDP_THREADING_SANITIZER_MODE:-}"

if [[ "${EDP_THREADING_SANITIZERS:-0}" == "1" && -z "${SANITIZER_MODE}" ]]; then
    SANITIZER_MODE="address,undefined"
fi

case "${SANITIZER_MODE}" in
    "")
        echo "EDP-Threading host foundation tests: compiling"
        compile_tests
        ;;
    address)
        echo "EDP-Threading host foundation tests: compiling (ASan)"
        compile_tests \
            -g \
            -fno-omit-frame-pointer \
            -fsanitize=address
        ;;
    undefined)
        echo "EDP-Threading host foundation tests: compiling (UBSan)"
        compile_tests \
            -g \
            -fno-omit-frame-pointer \
            -fsanitize=undefined
        ;;
    address,undefined)
        echo "EDP-Threading host foundation tests: compiling (ASan+UBSan)"
        compile_tests \
            -g \
            -fno-omit-frame-pointer \
            -fsanitize=address,undefined
        ;;
    *)
        echo "Unsupported EDP_THREADING_SANITIZER_MODE: ${SANITIZER_MODE}" >&2
        echo "Use: address, undefined, or address,undefined" >&2
        exit 2
        ;;
esac

echo "EDP-Threading host foundation tests: executing"

if [[ -n "${SANITIZER_MODE}" ]]; then
    ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0:abort_on_error=1}" \
    UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}" \
        "${OUTPUT}"
else
    "${OUTPUT}"
fi

echo "EDP-Threading host foundation tests: PASS"
