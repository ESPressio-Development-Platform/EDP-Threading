# Same-Worker native core migration

This hardware fixture uses one actual production Threading ordinary Worker, the native ESP-IDF execution owner and its native TLS carrier. The Worker has AnyAffinity and one immutable record with identity 9001. It is started once, executes a callable on core zero, resumes that same callable/task on core one, returns to its persistent Worker loop, executes another dispatch with the same binding, then retires and is joined/destroyed normally.

Two higher-priority pinned native load helpers force the scheduling transition. These are fixed-backed fixture infrastructure, not an alternate production scheduler. Their active load phase has a finite iteration cap. A separate pinned controller handles startup and shutdown. Helpers are deleted only after observed stable blocked state and with no remaining wake sender.

Build using the public GitHub dependencies in platformio.ini:

```sh
pio run -d tests/context-binding-migration -e esp32dev
pio run -d tests/context-binding-migration -e esp32s3
pio run -d tests/context-binding-migration -e esp32dev-arduino
pio run -d tests/context-binding-migration -e esp32s3-arduino
```

Choose the real device explicitly when uploading and capture serial at 115200 baud. Require the complete newline-terminated THREADING_MIGRATION_PASS same_worker=1 cores=0,1 identity=9001 retired=1 destroyed=1 line and reject any THREADING_MIGRATION_FAIL/panic. The final extra newline produces a short terminal USB packet. SDK compilation alone is not physical acceptance.

Each Worker/helper has an explicit 8192-byte stack. These are fixture capacities, not approved Profiling memory budgets or measured high-water bounds. Native helper calls and core inspection stay in this SDK-specific acceptance project; neutral Threading source remains free of SDK/Profiling dependencies. This test covers task identity and binding across one forced migration, not capture closure, exporter durability, arbitrary scheduler fairness, worst-case timing or cross-node clocks.

See ../context-binding-hil/README.MD for native failure/retention/incarnation qualification and ../../demos/current-context-binding for the complete consumer example.
