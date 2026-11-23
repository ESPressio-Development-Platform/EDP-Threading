# Typed current-context binding

This complete example selects Threading's optional typed current-context binding. It demonstrates one ordinary Worker on core 0, one isolated Dedicated Worker on core 1, and a restartable Dedicated Thread. Their immutable records retain identities across dispatch, managed blocking/yield and repeated activation. Startup is admitted only after every required record is bound; normal shutdown verifies exact retirement and native destruction.

This is a Threading integration example, not a complete Profiling capture/export implementation. Its three records and 8192-byte native stacks are explicit demonstration choices, not deployment defaults or accepted Profiling budgets.

## 1. Select a build form

Use PIOArduino with C++20 and select the checked-in project:

- [Arduino through PlatformIO/Pioarduino](./PlatformIO_Arduino/platformio.ini)
- [ESP-IDF through PlatformIO/Pioarduino](./PlatformIO_ESP-IDF/platformio.ini)
- [Arduino IDE sketch](./Arduino_IDE/Arduino_IDE.ino)

The PlatformIO projects declare every dependency using GitHub URLs. Threading and Platform select feature/profiling-v1; concrete Platform-ESP-IDF and synchronization Platform-FreeRTOS select main. No filesystem dependency roots or symlinks are required.

```sh
cd demos/current-context-binding/PlatformIO_Arduino
pio run -e esp32dev
pio run -e esp32s3
```

For ESP-IDF, use the adjacent PlatformIO_ESP-IDF project with the same environment names. For Arduino IDE, install the same repository revisions as libraries, select a current C++20-capable Arduino-ESP32 toolchain and open Arduino_IDE.ino with its sibling Example.hpp. The ESP32-S3 Arduino form enables hardware USB CDC.

## 2. Read the complete source

The entire integration is in [Example.hpp](./PlatformIO_Arduino/src/Example.hpp). The three copies are byte-identical. No generated-source script is needed. The Arduino entry is:

```cpp
#include <Arduino.h>
#include "Example.hpp"

void setup() { Example::Run(); }
void loop() { delay(1000); }
```

The ESP-IDF entry is:

```cpp
#include "Example.hpp"

extern "C" void app_main() { Example::Run(); }
```

Follow the source in order: select existing concrete providers; define a record; allocate stable record/mapping storage; declare the exact topology; explicitly add Policy as the seventh runtime template argument; construct the stable policy/runtime; bind one canonical Clock; initialize; start with an explicit finite duration; dispatch/activate work; begin cooperative shutdown; continue native finalization with a separately supplied finite duration.

## 3. Inspect startup and usage

`Observe` reads the actual typed Platform binding, validates its exact expected record/address/identity/slot and updates bounded application observation state under the existing SpinLock. It never asks Threading to scan its topology. Worker and Dedicated callbacks start only inside the bound persistent body.

`Workers.StartFor(Duration::FromNanoseconds(2000000000LL))` requires all native-entry acknowledgements before semantic dispatch/activation opens. The same record is observed before and after the Dedicated body's managed Wait, throughout three semantic activations and across repeated finite Worker calls. Unmanaged Bootstrap observes finite Task completion with a bounded canonical Clock check; it does not call a managed-context-only wait API from main.

## 4. Preserve ownership on failure

The executable demonstration stops with a FAIL marker on an unexpected contract outcome. In a real application, handle a failed `StartFor` result by keeping the runtime, policy, mapping and records alive, inspecting `Contexts.StartupResult()` / `Contexts.Snapshot(index)`, and calling `FinalizeShutdownFor` with an explicitly chosen continuation budget. If it does not return Completed, retain ownership and continue diagnosis/shutdown later. Never force-delete running native tasks, overwrite a foreign binding or reset activity flags to manufacture success.

A genuinely new native incarnation needs a fresh nonzero session identity and verified retirement of all prior native/capture/export references. Native retirement does not imply durable export completion. [Full lifecycle contract](../../docs/CONTEXT-BINDING.MD).

## 5. Check output

At 115200 baud a complete run prints:

```text
THREADING_BINDING_DEMO_PASS ordinary=3 isolated=3 dedicated=... policy_bytes=... runtime_bytes=...
```

Dedicated wake counts may exceed six because targeted wake notifications are advisory/latched. Actual byte counts are reported by the selected target/compiler. Firmware compilation alone is not evidence of device execution; qualification evidence is recorded separately in Policy handoffs.

## Qualified Arduino IDE toolchain

The actual sketch was compiled and run on ESP32-WROOM and ESP32-S3 with official Arduino CLI 1.6.0 and Espressif Arduino-ESP32 core 4.0.0, with C++20 explicitly selected. Install the same linked library revisions in the configured Arduino library location before compiling.

From this example directory:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@4.0.0 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli compile --fqbn esp32:esp32:esp32 --build-property compiler.cpp.extra_flags=-std=gnu++20 Arduino_IDE
arduino-cli compile --fqbn esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc --build-property compiler.cpp.extra_flags=-std=gnu++20 Arduino_IDE
```

The IDE/core-4 sample reports Policy=120 bytes and Runtime=27384 bytes; the separately qualified PIO profiles report 112 and 27232 bytes. These are explicit example configurations on those exact SDKs, not a universal footprint or approved Profiling budget. Passing the example does not qualify arbitrary SDKs or the unfinished Profiling capture/export system.
