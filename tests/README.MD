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

The runner compiles `ThreadingFoundationTests.cpp` as C++17 with `-Wall -Wextra -Wpedantic -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer enabled by default, executes it, and removes the temporary binary. Set `EDP_THREADING_SANITIZERS=0` only when an environment cannot provide those host sanitizers.

## ESP32 integration demo

The buildable integration demo is under `demos/basic-threading`.

For PlatformIO Arduino:

```bash
cd demos/basic-threading/PlatformIO_Arduino
pio run
pio run --target upload
pio device monitor
```

For PlatformIO ESP-IDF:

```bash
cd demos/basic-threading/PlatformIO_ESP-IDF
pio run
pio run --target upload
pio device monitor
```

Successful execution prints:

```
EDP-Threading demo: PASS
```

The Arduino IDE variant is in `demos/basic-threading/Arduino_IDE` and requires the EDP libraries used by the PlatformIO manifests to be installed in the Arduino library search path.
