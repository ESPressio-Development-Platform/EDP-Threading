# Threading Architecture

## Purpose

EDP-Threading owns two execution models: finite `Task<TResult>` work executed by bounded Task facilities, and persistent topology-owned Dedicated Threads controlled through `Thread<TThreadIdentity>`.

Tasks do not intrinsically own native execution contexts. Workers are internal reusable contexts. Dedicated Threads own independent persistent Platform execution contexts and never consume Task Worker capacity.

## Platform boundary

Threading consumes `EDP-Platform` execution and synchronization mechanisms. It does not expose FreeRTOS, ESP-IDF, Arduino or operating-system vocabulary through its public execution semantics.

Logical execution priority is Low, Normal, High or Critical. Processor affinity is expressed through Platform's logical affinity abstraction.

## Time boundary

All monotonic deadlines and physical-duration calculations derive from the application-selected canonical `EDP-Clock` monotonic timeline. Native scheduler ticks are implementation mechanisms only.

## Composition

`ThreadingTopology<...>` describes statically bounded execution resources. A topology offers TaskExecution only when a Task facility exists and DedicatedThreadExecution only when a Dedicated Thread exists.

Application Bootstrap owns runtime realization and lifecycle. Composition validates architecture; it is not a runtime service locator or dependency-injection container.
