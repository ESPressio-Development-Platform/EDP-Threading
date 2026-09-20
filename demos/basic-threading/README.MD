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

PlatformIO manifests declare every required EDP repository explicitly. The Arduino IDE variant requires those libraries to be installed in the Arduino library search path.

The PlatformIO ESP-IDF surface pins `pioarduino/platform-espressif32` to commit `d41d8383706dc146fd91a0e1d3c59a915a4d493e`. The pioarduino stable 55.3.311 package conflicts with PlatformIO Core 6.2.x because it declares SCons 4.8.1 while Core manages SCons 4.11.1; the pinned revision aligns the platform package with SCons 4.11.1 while retaining ESP-IDF 5.5.5.

That pioarduino revision currently declares `tool-esptoolpy` 5.4.0, whose progress logger can fail during upload with `AttributeError: 'EsptoolLogger' object has no attribute '_get_progress_print_file'` after the board and flashing stub have already connected successfully. The demo therefore overrides only `tool-esptoolpy` to pioarduino's preceding 5.3.1 package. This keeps the compiler/framework baseline unchanged while avoiding the uploader regression.
