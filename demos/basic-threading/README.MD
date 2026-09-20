# Basic Threading Demo

This demo exercises one bounded Task facility and one persistent Dedicated Thread using the concrete FreeRTOS Platform providers on ESP32.

Expected serial/console output after successful execution:

```
EDP-Threading demo: PASS
```

The demo deliberately uses `AbandonImmediately` for Task admission and polls terminal state from the application context, so it does not require application monotonic-clock binding merely to demonstrate the core execution lifecycle.

It validates the complete consumer lifecycle: static topology construction, initialization, transactional infrastructure Start, Task dispatch/result extraction, Dedicated Thread activation/cooperative stop, terminal shutdown, infrastructure finalization and shutdown-completion observation.

## Concrete providers

The Arduino targets use the vanilla FreeRTOS execution, signal and mutex providers plus the portable lock-free AtomicWord8 provider. The ESP-IDF target uses the ESP-IDF execution provider (including its native affinity-capable implementation), the FreeRTOS signal/mutex providers and the portable AtomicWord8 provider.

PlatformIO manifests declare every required EDP repository explicitly. The Arduino IDE variant requires those libraries to be installed in the Arduino library search path.
