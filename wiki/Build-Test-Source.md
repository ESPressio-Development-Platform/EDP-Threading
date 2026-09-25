# Build, Test and Source Map

C++20 is required. `docs/ARCHITECTURE.MD`, `docs/LIFECYCLE.MD` and `docs/RESOURCES.MD` are durable design references.

Host validation requires sibling source trees for EDP-System, EDP-Platform, EDP-Clock and EDP-BoundedTopology. The warnings-as-errors GitHub host workflows checkout the same mandatory dependencies explicitly.

UBSan, Arduino/ESP-IDF compile-link validation and resource-measurement projects protect topology, lifecycle, cancellation, waits and deterministic RAM behaviour. The coherent local-source resource runner additionally checks out/includes EDP-BoundedTopology so migration measurements use one consistent source graph.

`demos/basic-threading` is the primary integration demonstration.
