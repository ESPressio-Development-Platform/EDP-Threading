# Compiler Definitions and Conditional Compilation

## Repository-defined compiler definition

### `EDP_THREADING_MEASUREMENT_SCENARIO`

**Scope:** test/resource-measurement infrastructure only. It is not a production Threading configuration knob.

The definition is supplied by `tests/resource-measurements/platformio.ini` and consumed by `tests/resource-measurements/src/main.cpp`.

| Value | Measurement scenario |
|---:|---|
| `0` | Framework baseline; Threading topology/resource scenario disabled |
| `1` | Small ordinary Task facility: 8 records, one worker |
| `2` | Representative ordinary Task facility: 16 records, two workers |
| `3` | Higher-priority control Task facility |
| `4` | Dedicated Worker lease |
| `5` | Dedicated Thread |
| `6` | Mixed topology: two ordinary facilities, one dedicated worker and two dedicated threads |

Both ESP-IDF and Arduino resource-measurement environments use the same scenario values so their retained-resource measurements are comparable.

## Platform/toolchain-provided conditional

### `ARDUINO`

The resource-measurement source uses the framework-provided `ARDUINO` macro to choose the native ExecutionContext provider:

- defined: `EDP-Platform-FreeRTOS::ExecutionContextProvider`;
- otherwise: `EDP-Platform-ESP-IDF::ExecutionContextProvider`.

This macro is supplied by the Arduino build environment; EDP-Threading does not define it.

## Production library

No repository-defined production compiler definition is required to configure the EDP-Threading public/runtime API. Threading topology, capacities, priorities and affinity are expressed through C++ types/templates rather than preprocessor configuration.

> Audited against `2c1a7d7d5c50eb8b40ee7c13fbb46cb797c53182` on `main`.
