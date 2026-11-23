#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PIO="${PIO:-pio}"
if [[ $# -gt 1 ]]; then
    echo 'Usage: run-context-binding-demo-builds.sh [esp32dev|esp32s3]' >&2
    exit 2
fi
targets=(esp32dev esp32s3)
if [[ $# -eq 1 ]]; then
    case "$1" in
        esp32dev|esp32s3) targets=("$1") ;;
        *) echo 'Unsupported demo target' >&2; exit 2 ;;
    esac
fi
for framework in PlatformIO_Arduino PlatformIO_ESP-IDF; do
    for target in "${targets[@]}"; do
        "$PIO" run -d "$ROOT/demos/current-context-binding/$framework" -e "$target"
    done
done
