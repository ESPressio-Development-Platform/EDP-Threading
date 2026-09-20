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

The PlatformIO ESP-IDF surface pins `pioarduino/platform-espressif32` to commit `2f8e8ac3931840b7e9767b79b61184eb29100e3c`, the last revision whose native uploader package is the coherent `esptoolpy 5.3.1` archive while still using ESP-IDF 5.5.5. PlatformIO Core 6.2.x requires SCons 4.11.1, so the demo overrides only `tool-scons` to pioarduino's 4.11.1 package.

This combination is intentional. The later pioarduino SCons-4.11.1 revision changed only `platform.json`; its builder code is identical to the pinned 5.3.1-uploader revision. Keeping the platform's native esptool package avoids the broken mixed-package state where an esptool launcher exists in the shared pioarduino Python environment but the downgraded module itself is not importable.
