Import("env")

import os
import shlex
import subprocess


MEASUREMENT_SYMBOLS = {
    "intrinsic": "EDP_Threading_Measurement_Intrinsic",
    "provider_objects": "EDP_Threading_Measurement_ProviderObjects",
    "control_backing": "EDP_Threading_Measurement_ControlBacking",
    "stack_backing": "EDP_Threading_Measurement_StackBacking",
    "sync_wake": "EDP_Threading_Measurement_SynchronizationWake",
}


def resolve_tool(command):
    executable = shlex.split(
        env.subst(command)
    )

    if len(executable) != 1:
        raise RuntimeError(
            "Expected one tool executable for " + command
        )

    return executable[0]


def resolve_nm_tool():
    size_tool = resolve_tool(
        "$SIZETOOL"
    )

    tool_root, extension = os.path.splitext(
        size_tool
    )

    if not tool_root.endswith(
        "-size"
    ):
        raise RuntimeError(
            "Unable to derive nm tool from SIZETOOL: " +
            size_tool
        )

    return (
        tool_root[:-5] +
        "-nm" +
        extension
    )


def run_tool(executable, arguments):
    process_environment = os.environ.copy()
    process_environment.update(
        {
            str(key): str(value)
            for key, value in env.get(
                "ENV",
                {},
            ).items()
        }
    )

    completed = subprocess.run(
        [
            executable,
            *arguments,
        ],
        check=True,
        capture_output=True,
        text=True,
        env=process_environment,
    )

    return completed.stdout


def parse_size(elf_path):
    output = run_tool(
        resolve_tool(
            "$SIZETOOL"
        ),
        [
            "-B",
            "-d",
            elf_path,
        ],
    )

    lines = [
        line.strip()
        for line in output.splitlines()
        if line.strip()
    ]

    if len(lines) < 2:
        raise RuntimeError(
            "Unable to parse target size output"
        )

    fields = lines[-1].split()

    if len(fields) < 3:
        raise RuntimeError(
            "Unable to parse target text/data/bss sizes"
        )

    return {
        "text": int(fields[0]),
        "data": int(fields[1]),
        "bss": int(fields[2]),
    }


def parse_measurement_symbols(elf_path):
    output = run_tool(
        resolve_nm_tool(),
        [
            "-S",
            elf_path,
        ],
    )

    sizes = {}

    for line in output.splitlines():
        fields = line.split()

        if len(fields) < 4:
            continue

        symbol = fields[-1]

        for metric, expected_symbol in MEASUREMENT_SYMBOLS.items():
            if symbol == expected_symbol:
                sizes[metric] = int(
                    fields[1],
                    16,
                )

    missing = [
        metric
        for metric in MEASUREMENT_SYMBOLS
        if metric not in sizes
    ]

    if missing:
        raise RuntimeError(
            "Missing resource-measurement symbols: " +
            ", ".join(
                missing
            )
        )

    return sizes


def report_measurement(target, source, env):
    elf_path = target[0].get_abspath()
    environment = env.subst(
        "$PIOENV"
    )

    target_size = parse_size(
        elf_path
    )

    if environment.endswith(
        "_baseline"
    ):
        print(
            "EDP_THREADING_BASELINE"
            f" env={environment}"
            f" text={target_size['text']}"
            f" data={target_size['data']}"
            f" bss={target_size['bss']}"
        )
        return

    measurements = parse_measurement_symbols(
        elf_path
    )

    total = sum(
        measurements.values()
    )

    print(
        "EDP_THREADING_MEASUREMENT"
        f" env={environment}"
        f" total={total}"
        f" intrinsic={measurements['intrinsic']}"
        f" provider_objects={measurements['provider_objects']}"
        f" control_backing={measurements['control_backing']}"
        f" stack_backing={measurements['stack_backing']}"
        f" sync_wake={measurements['sync_wake']}"
        f" text={target_size['text']}"
        f" data={target_size['data']}"
        f" bss={target_size['bss']}"
    )


env.AddPostAction(
    "$PROGPATH",
    report_measurement,
)
