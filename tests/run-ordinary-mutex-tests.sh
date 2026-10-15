#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="$(cd "${ROOT}/.." && pwd)"
OUTPUT="${ROOT}/tests/.threading-ordinary-mutex-tests"
trap 'rm -f "${OUTPUT}"' EXIT

for dependency in EDP-System EDP-Platform EDP-Clock EDP-BoundedTopology EDP-Memory; do
    if [[ ! -d "${WORKSPACE}/${dependency}/src" ]]; then
        echo "Missing sibling repository ${dependency}" >&2
        exit 2
    fi
done

"${CXX:-c++}" -std=c++20 -Wall -Wextra -Wpedantic -Werror \
    -I"${ROOT}/src" \
    -I"${WORKSPACE}/EDP-System/src" \
    -I"${WORKSPACE}/EDP-Platform/src" \
    -I"${WORKSPACE}/EDP-Clock/src" \
    -I"${WORKSPACE}/EDP-BoundedTopology/src" \
    -I"${WORKSPACE}/EDP-Memory/src" \
    "${ROOT}/tests/OrdinaryMutexTests.cpp" \
    -o "${OUTPUT}"

"${OUTPUT}"
echo "EDP-Threading ordinary mutex tests: PASS"
