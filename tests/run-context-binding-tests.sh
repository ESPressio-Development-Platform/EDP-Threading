#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="$(dirname "$ROOT")"
OUTPUT="$(mktemp "${TMPDIR:-/tmp}/edp-context-binding-XXXXXX")"
trap 'rm -f "$OUTPUT"' EXIT
includes=(-I"$ROOT/src")
for dependency in EDP-System EDP-Platform EDP-Clock EDP-BoundedTopology EDP-Memory EDP-Platform-Portable; do
    includes+=(-I"$WORKSPACE/$dependency/src")
done
flags=("${EDP_THREADING_TEST_OPTIMIZATION:--O0}" -std=c++20 -Wall -Wextra -Wpedantic -Werror -pthread)
if [[ "${EDP_THREADING_SANITIZERS:-0}" == 1 ]]; then
    flags+=(-g -fno-omit-frame-pointer -fsanitize=address,undefined)
fi
"${CXX:-c++}" "${flags[@]}" "${includes[@]}" "$ROOT/tests/ContextBindingTests.cpp" -o "$OUTPUT"
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1 UBSAN_OPTIONS=halt_on_error=1 timeout 30 "$OUTPUT"

ASAN_OPTIONS=detect_leaks=0:abort_on_error=1 UBSAN_OPTIONS=halt_on_error=1 timeout 30 "$OUTPUT" --exception
