#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORKSPACE="$(cd "${ROOT}/.." && pwd)"

required=(
    EDP-System EDP-Platform EDP-Clock EDP-BoundedTopology EDP-Memory
    EDP-Platform-FreeRTOS EDP-Platform-ESP-IDF EDP-Threading
)

for repository in "${required[@]}"; do
    if [[ ! -d "${WORKSPACE}/${repository}/src" ]]; then
        echo "Missing sibling repository: ${WORKSPACE}/${repository}" >&2
        exit 2
    fi
done
build_demo() {
    local project="$1"
    local source_conf="${ROOT}/${project}/platformio.ini"
    local temp_conf
    temp_conf="$(mktemp "${ROOT}/${project}/platformio.local.XXXXXX.ini")"

    python3 - "${source_conf}" "${temp_conf}" "${WORKSPACE}" <<'PY'
from pathlib import Path
import sys

source, target, workspace = map(Path, sys.argv[1:])
lines = source.read_text().splitlines()
output = []
skipping_deps = False
for line in lines:
    if line == "lib_deps =":
        skipping_deps = True
        continue
    if skipping_deps:
        if line.startswith("["):
            skipping_deps = False
        else:
            continue
    output.append(line)

include_flags = [
    f"    -I{workspace / repository / 'src'}"
    for repository in [
        "EDP-System", "EDP-Platform", "EDP-Clock", "EDP-BoundedTopology",
        "EDP-Memory", "EDP-Platform-FreeRTOS", "EDP-Platform-ESP-IDF",
        "EDP-Threading",
    ]
]

expanded = []
for line in output:
    expanded.append(line)
    if line == "build_flags =":
        expanded.extend(include_flags)

target.write_text("\n".join(expanded) + "\n")
PY

    rm -rf "${ROOT}/${project}/.pio"
    pio run -d "${ROOT}/${project}" --project-conf "${temp_conf}"
    rm -f "${temp_conf}"
}
echo "EDP-Threading basic demo builds: Arduino"
build_demo "demos/basic-threading/PlatformIO_Arduino"

echo "EDP-Threading basic demo builds: ESP-IDF"
build_demo "demos/basic-threading/PlatformIO_ESP-IDF"

echo "EDP-Threading basic demo builds: PASS"
