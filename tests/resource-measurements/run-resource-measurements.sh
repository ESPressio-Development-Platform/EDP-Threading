#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIRECTORY="${ROOT}/.measurement-logs"
PROJECT_CONFIG="${ROOT}/platformio.ini"
ACTIVE_CONFIG="${PROJECT_CONFIG}"
TEMP_CONFIG=""

cleanup() {
    if [[ -n "${TEMP_CONFIG}" && -f "${TEMP_CONFIG}" ]]; then
        rm -f "${TEMP_CONFIG}"
    fi
}

trap cleanup EXIT INT TERM

rm -rf "${ROOT}/.pio"
rm -rf "${LOG_DIRECTORY}"

mkdir -p "${LOG_DIRECTORY}"

if [[ -n "${EDP_THREADING_RESOURCE_LOCAL_ROOT:-}" ]]; then
    LOCAL_ROOT="$(cd "${EDP_THREADING_RESOURCE_LOCAL_ROOT}" && pwd)"

    required_repositories=(
        EDP-System
        EDP-Platform
        EDP-Memory
        EDP-Security
        EDP-Clock
        EDP-Platform-FreeRTOS
        EDP-Platform-ESP-IDF
        EDP-Threading
    )

    for repository in "${required_repositories[@]}"; do
        if [[ ! -f "${LOCAL_ROOT}/${repository}/library.json" ]]; then
            echo "ERROR: missing local resource-measurement dependency: ${LOCAL_ROOT}/${repository}/library.json" >&2
            exit 1
        fi
    done

    TEMP_CONFIG="$(mktemp "${ROOT}/platformio.local.XXXXXX.ini")"

    python3 - "${PROJECT_CONFIG}" "${TEMP_CONFIG}" "${LOCAL_ROOT}" <<'PY'
from pathlib import Path
import sys

source = Path(sys.argv[1])
target = Path(sys.argv[2])
local_root = Path(sys.argv[3])

text = source.read_text()

start = text.index("lib_deps =\n")
end = text.index("\n\n[env:idf_baseline]", start)

repositories = [
    "EDP-System",
    "EDP-Platform",
    "EDP-Memory",
    "EDP-Security",
    "EDP-Clock",
    "EDP-Platform-FreeRTOS",
    "EDP-Platform-ESP-IDF",
    "EDP-Threading",
]

local_deps = "lib_deps =\n" + "".join(
    f"    {repository}=symlink://{local_root / repository}\n"
    for repository in repositories
)

target.write_text(text[:start] + local_deps + text[end:])
PY

    ACTIVE_CONFIG="${TEMP_CONFIG}"

    echo "EDP-Threading resource measurements: local dependency mode"
    echo "Local ESPressio root: ${LOCAL_ROOT}"
fi

environments=(
    idf_baseline
    idf_small_8x1
    idf_representative_16x2
    idf_control_high
    idf_dedicated_worker
    idf_dedicated_thread
    idf_mixed
    arduino_baseline
    arduino_small_8x1
    arduino_representative_16x2
    arduino_control_high
    arduino_dedicated_worker
    arduino_dedicated_thread
    arduino_mixed
)

for environment in "${environments[@]}"; do
    echo
    echo "=== ${environment} ==="

    (
        cd "${ROOT}"
        pio run \
            --project-conf "${ACTIVE_CONFIG}" \
            --environment "${environment}"
    ) | tee "${LOG_DIRECTORY}/${environment}.log"
done

echo
echo "EDP-Threading resource measurement summary"
echo "=========================================="

for environment in "${environments[@]}"; do
    grep \
        -E "EDP_THREADING_(BASELINE|MEASUREMENT)" \
        "${LOG_DIRECTORY}/${environment}.log"
done
