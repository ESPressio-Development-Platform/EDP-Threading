# Build, Test and Source Map

C++20 is required. `docs/ARCHITECTURE.MD`, `docs/LIFECYCLE.MD` and `docs/RESOURCES.MD` are durable design references.

Host validation requires sibling source trees for EDP-System, EDP-Platform, EDP-Clock, EDP-BoundedTopology and EDP-Memory. The warnings-as-errors GitHub host workflows checkout the same mandatory dependencies explicitly.

UBSan, Arduino/ESP-IDF compile-link validation and resource-measurement projects protect topology, lifecycle, cancellation, waits and deterministic RAM behaviour. The coherent local-source resource runner additionally checks out/includes EDP-BoundedTopology so migration measurements use one consistent source graph.

The bounded-topology migration completion run, performed immediately before reintegration into `main`, passed warnings-as-errors host execution, Host foundation, UBSan, and all fourteen resource-measurement environments. Its strict comparison found byte-for-byte identical `total` and `intrinsic` retained RAM versus the validated 23 September baseline.

`demos/basic-threading` is the primary integration demonstration.

The Event-support tranche adds explicit host tests for `Thread::Wake`, indefinite/finite `ThreadContext` waits, moved-from wake safety, and keyed ordinary mutex success/provider failure. The ESP32 basic-threading demo exercises an indefinitely waiting Dedicated Thread, external wake, stop wake and the mutex adapter.



## Optional context binding

tests/run-context-binding-tests.sh compiles actual Threading ownership over independent host threads, runs precise failure/lifetime cases and a separate exceptional-exit subprocess. These fixtures are not native ESP32 evidence; SDK/device results are recorded independently in Policy handoffs. See [contract and reference](Context-Binding).

## Complete optional-binding demo projects

The current-context-binding demo has identical complete Example.hpp sources for Arduino IDE and both PIOArduino framework projects. tests/run-context-binding-demo-builds.sh builds both declared target environments under Arduino and ESP-IDF, or accepts one explicit target. Public manifests use GitHub URLs, without filesystem dependency roots. See [runner reference](Reference-tests-run-context-binding-demo-builds) and [project walkthrough](../demos/current-context-binding/README.MD). Device execution evidence remains separate from compilation.


Actual Arduino IDE/CLI source discovery and compilation were exercised with CLI1.6 and core4 on both boards; the merged sketch images also ran physically on both targets. The four PIO profiles are separate SDK evidence. The complete native failure/stress/migration acceptance remains a separate fixture rather than a claim inferred from the basic demo.
