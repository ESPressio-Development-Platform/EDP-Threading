# Basic Threading Demo

This demo exercises one bounded Task facility and one persistent Dedicated Thread using the concrete FreeRTOS Platform providers on ESP32.

Expected serial/console output after successful execution:

```
EDP-Threading demo: PASS
```

The demo deliberately uses `AbandonImmediately` for Task admission and polls terminal state from the application context, so it does not require application monotonic-clock binding merely to demonstrate the core execution lifecycle.

It validates the complete consumer lifecycle: static topology construction, initialization, transactional infrastructure Start, Task dispatch/result extraction, Dedicated Thread activation/cooperative stop, terminal shutdown, infrastructure finalization and shutdown-completion observation.

## Concrete providers

The Arduino targets use the vanilla FreeRTOS execution, signal and mutex providers plus the ESP-IDF `SpinLockProvider` for the single topology-wide lifecycle byte. The ESP-IDF target uses the ESP-IDF execution provider (including its native affinity-capable implementation), the FreeRTOS signal/mutex providers and the same ESP-IDF spin lock. Per-Task and per-Dedicated-Thread intrinsic control remains exactly one byte and is serialized through the mutex already owned by the corresponding runtime. This keeps lifecycle publication race-safe without requiring a target-wide lock-free atomic guarantee that the Xtensa toolchain does not advertise.

PlatformIO manifests declare every required EDP repository explicitly. For current-tip validation, remove the selected demo's local `.pio` directory before building so the EDP `#main` dependencies are freshly resolved instead of being reused from PlatformIO's cached checkout. The Arduino IDE variant requires those libraries to be installed in the Arduino library search path.

The PlatformIO ESP-IDF surface pins `pioarduino/platform-espressif32` to commit `d41d8383706dc146fd91a0e1d3c59a915a4d493e`. That revision is the compile/link-validated combination for PlatformIO Core 6.2.x, SCons 4.11.1 and ESP-IDF 5.5.5.

The PlatformIO Arduino and ESP-IDF surfaces currently encounter a host-side esptool 5.4.0 progress-rendering regression on this upload path: flashing reaches the ESP32, starts the stub flasher and begins the write, then the Python logger raises `AttributeError: 'EsptoolLogger' object has no attribute '_get_progress_print_file'`.

Esptool defines `--no-progress` on the `write-flash` subcommand, not as a global option. PlatformIO's ordinary `upload_flags` are placed before `write-flash`, which makes esptool reject the option. Each PlatformIO demo therefore contains `scripts/esptool_no_progress.py`, a small post-script that inserts `--no-progress` immediately after `write-flash` in `UPLOADERFLAGS`. This keeps the compile/link-validated uploader and framework packages unchanged while bypassing only the broken progress renderer. The ESP-IDF surface was physically flash/run validated with this workaround before the final V2 compliance cleanup changed Threading/provider production surfaces. That result remains useful historical evidence, but fresh physical validation against the current tips is still required before completion is claimed.

## C++ Language Baseline

Both PlatformIO-compatible surfaces compile as C++20. The Arduino surface selects maintained PIOArduino stable explicitly. The ESP-IDF surface intentionally retains the previously compile/link-validated PIOArduino commit pin `d41d8383706dc146fd91a0e1d3c59a915a4d493e` so its known PlatformIO Core/SCons/ESP-IDF compatibility is preserved.
