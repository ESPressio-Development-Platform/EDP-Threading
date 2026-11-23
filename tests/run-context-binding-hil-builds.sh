#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PIO="${PIO:-pio}"
targets=(esp32dev esp32s3 esp32dev-arduino esp32s3-arduino)
if [[ $# -gt 0 ]]; then
    targets=("$@")
fi
for target in "${targets[@]}"; do
    case "$target" in
        esp32dev|esp32s3|esp32dev-arduino|esp32s3-arduino) ;;
        *) echo 'Unsupported context-binding acceptance target' >&2; exit 2 ;;
    esac
    "$PIO" run -d "$ROOT/tests/context-binding-hil" -e "$target"
    "$PIO" run -d "$ROOT/tests/context-binding-migration" -e "$target"
done
