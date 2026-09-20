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

c++ \
    -std=c++17 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Werror \
    -I"${ROOT}/src" \
    -I"${WORKSPACE}/EDP-System/src" \
    -I"${WORKSPACE}/EDP-Platform/src" \
    -I"${WORKSPACE}/EDP-Clock/src" \
    "${ROOT}/tests/ThreadingFoundationTests.cpp" \
    -o "${OUTPUT}"

"${OUTPUT}"

echo "EDP-Threading host foundation tests: PASS"
