# EDP-Threading Developer Wiki

EDP-Threading owns deterministic execution-domain semantics for bounded Tasks, isolated workers and persistent Dedicated Threads over lower-level Platform execution/synchronization and shared bounded-topology mechanics.

This Wiki is maintained beside the code on the same branch. Source code and repository `docs/` remain normative; the Wiki is the internal developer explanation/navigation layer and must evolve with code changes.

## Public entry point

```cpp
#include <ESPressio_Threading.hpp>
```

## Dependencies

Mandatory: EDP-System, EDP-Platform, EDP-Clock and EDP-BoundedTopology.

EDP-BoundedTopology supplies the compact bounded-index, one-bit membership-set and intrusive-FIFO mechanics. Threading retains the Task/Worker semantic meaning, lifecycle, scheduling and synchronization layered over those primitives.

Use [Architecture](Architecture.md), [Public API](Public-API.md), [Internal API](Internal-API.md), [Implementation](Implementation.md), [Composition](Composition.md), [Resources / Lifecycle / Concurrency](Resources-Lifecycle-Concurrency.md), and [Build / Test / Source](Build-Test-Source.md).
