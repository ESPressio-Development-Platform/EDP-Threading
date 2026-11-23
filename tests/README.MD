# Local validation

## Host foundation suite

Clone these repositories as siblings:

```
EDP-System/
EDP-Platform/
EDP-Clock/
EDP-Threading/
```

Then, from `EDP-Threading`:

```bash
bash tests/run-host-tests.sh
```

The runner compiles `ThreadingFoundationTests.cpp` as C++20 with `-Wall -Wextra -Wpedantic -Werror`, executes it, then executes the focused `FailClosedOrdinaryMutexTests.cpp` suite. That focused suite covers healthy ownership, acquire failure, release failure, sticky terminal state, and a caller already waiting when another caller discovers release failure. Temporary binaries are removed. Runtime progress markers are always emitted.

Run the current supported sanitizer validation with:

```bash
EDP_THREADING_SANITIZER_MODE=undefined bash tests/run-host-tests.sh
```

AddressSanitizer modes remain available through `EDP_THREADING_SANITIZER_MODE=address` or `address,undefined`, but on the authoritative RPI400 appliance the ASan primary allocator currently aborts during initialization before any Threading test reaches `main()`. That behavior is treated as an appliance/toolchain limitation, not as Threading PASS or failure evidence.

## ESP32 integration demo

The buildable integration demo is under `demos/basic-threading`.

For PlatformIO Arduino:

```bash
cd demos/basic-threading/PlatformIO_Arduino
rm -rf .pio
pio run
pio run --target upload
pio device monitor
```

For PlatformIO ESP-IDF:

```bash
cd demos/basic-threading/PlatformIO_ESP-IDF
rm -rf .pio
pio run
pio run --target upload
pio device monitor
```

Removing the local `.pio` directory before each current-tip validation forces fresh resolution of the EDP `#main` dependencies instead of reusing cached PlatformIO library checkouts.

Successful execution prints:

```
EDP-Threading demo: PASS
```

The Arduino IDE variant is in `demos/basic-threading/Arduino_IDE` and requires the EDP libraries used by the PlatformIO manifests to be installed in the Arduino library search path.

For authoritative branch-coherent build validation on RPI400, run:

```bash
bash tests/run-basic-demo-builds.sh
```

The runner injects the sibling repository `src` directories directly so PlatformIO cannot silently substitute remote `main` packages for the branch topology under test.

## Keyed ordinary mutex finite/NoWait prerequisite

The host runner also invokes `tests/run-ordinary-mutex-tests.sh`, compiling/executing `tests/OrdinaryMutexTests.cpp`. That dedicated suite checks immediate contention, relative finite timeout forwarding, distinctly injected provider failure, legacy Forever acquisition, release, and zero added Timebase/heap. The native relative timeout is only the mechanism: an EDP-Logging caller must still pass its *current remaining* canonical EDP-Clock deadline budget.

**Status:** test code is published to GitHub but **not yet executed** on RPI400 or AI-AGENT-02. Do not infer D12.1 is fully met by this Threading extension alone; EDP-Memory's internal lock still needs a bounded mechanism.


## Optional context binding

Run `bash tests/run-context-binding-tests.sh` with coherent sibling Platform and Portable `feature/profiling-v1` plus System, Clock, Memory and BoundedTopology `main`. `CXX` selects GCC/Clang; `EDP_THREADING_TEST_OPTIMIZATION` selects the compiler optimization flag; `EDP_THREADING_SANITIZERS=1` enables ASan/UBSan. The runner uses a temporary executable, preserves compiler/test exit status and imposes a 30-second host fixture limit. It does not run ESP32 firmware.

The actual production owner is exercised with real host threads and deliberately synthetic carrier/native fixtures. Checks cover ordinary and dedicated Workers, persistent Dedicated activation, compile-time explicit budget requirements, exact binding rejection, partial multi-Worker startup, late and exhausted acknowledgement budgets, repeated fresh incarnations, invalid mappings, all retirement anomalies and exceptional owner exit with retained storage. The exceptional subprocess intentionally terminates while its failed owner is retained; this proves refusal, not production reclamation.

Run `bash tests/run-context-binding-demo-builds.sh` to build both native boards under Arduino and ESP-IDF. An optional `esp32dev` or `esp32s3` argument restricts the target; `PIO` selects the executable. The complete examples use GitHub dependency URLs. This runner does not flash hardware or claim Arduino IDE build results.
