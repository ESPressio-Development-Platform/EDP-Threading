#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIRECTORY="${ROOT}/.measurement-logs"

rm -rf "${ROOT}/.pio"
rm -rf "${LOG_DIRECTORY}"

mkdir -p "${LOG_DIRECTORY}"

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
        pio run             --environment "${environment}"
    ) | tee "${LOG_DIRECTORY}/${environment}.log"
done

echo
echo "EDP-Threading resource measurement summary"
echo "=========================================="

for environment in "${environments[@]}"; do
    grep         -E "EDP_THREADING_(BASELINE|MEASUREMENT)"         "${LOG_DIRECTORY}/${environment}.log"
done
