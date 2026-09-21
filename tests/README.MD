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

The runner compiles `ThreadingFoundationTests.cpp` as C++20 with `-Wall -Wextra -Wpedantic -Werror`, executes it, and removes the temporary binary. Runtime progress markers are always emitted.

Run the current supported sanitizer validation with:

```bash
EDP_THREADING_SANITIZER_MODE=undefined bash tests/run-host-tests.sh
```

AddressSanitizer modes remain available through `EDP_THREADING_SANITIZER_MODE=address` or `address,undefined`, but on the validating macOS host ASan currently hangs before `main()`. That behavior is treated as a host sanitizer/runtime limitation, not as Threading PASS or failure evidence.

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
