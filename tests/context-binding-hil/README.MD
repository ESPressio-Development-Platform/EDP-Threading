# Native context-binding acceptance fixture

This project runs the production Threading owner and ESP-IDF execution/TLS providers on ESP32 and ESP32-S3. It deliberately injects failures through forwarding adapters; these adapters are test code.

Build with PlatformIO and the GitHub dependency references in platformio.ini:

```sh
pio run -d tests/context-binding-hil -e esp32dev
pio run -d tests/context-binding-hil -e esp32s3
pio run -d tests/context-binding-hil -e esp32dev-arduino
pio run -d tests/context-binding-hil -e esp32s3-arduino
```

Select the actual device explicitly before uploading. Capture serial at 115200 baud. A successful run prints 25 THREADING_BINDING_HIL_CASE_PASS lines, THREADING_BINDING_HIL_RETIREMENT_RETAINED_PASS and the complete newline-terminated THREADING_BINDING_HIL_PASS incarnations=26 native_contexts=104 footer. Require every marker and reject any failure/panic marker. The extra final newline keeps the terminal USB packet short on the qualified S3 Arduino profile; preserved earlier captures reached all individual assertions but omitted the exactly 64-byte final line. The underlying transport cause has not been proven.

The four contexts comprise two ordinary Workers, one isolated Worker and one persistent Dedicated thread. The fixture checks repeated dispatch and activation, a pre-existing foreign native binding, explicit UnsupportedContext, second-Worker native Start refusal, startup timeout with a gated late native entry, zero-budget cleanup retaining that entry, and cooperative cleanup after release. Twenty additional owner incarnations reuse one fixed caller-owned storage block through EDP-Memory ObjectLifetime with fresh nonzero identities. Every new native entry verifies genuine TLS starts unbound.

The final scenario replaces the Dedicated record with a fixture-owned foreign record. Actual native Join succeeds, but the production policy retains RetirementDifferentBinding and refuses Destroy/normal completion. This final owner, mapping, records and backing intentionally remain alive until firmware reset. A PASS establishes correct refusal and retention; it does not establish reclamation of that failed owner. Only the fixture that installed the foreign record retires it.

The fixture uses four explicit 4096-byte stacks, native ESP-IDF execution, existing FreeRTOS Signal/Mutex, native SpinLock and the canonical Clock. Stack capacities are test configuration, not deployment budgets or high-water proofs. No private TLS, scheduler, allocator or clock is introduced. Threading's own startup/cleanup waits are finite; the fixture-only late-entry gate is deliberately released by its controller.

This fixture does not force one Worker to migrate cores, qualify exceptions under the SDK, exercise capture/export/archive durability, or prove full Disabled Profiling removal. Record those acceptance items separately. Host coverage is in ../run-context-binding-tests.sh; the complete consumer example is in ../../demos/current-context-binding.
