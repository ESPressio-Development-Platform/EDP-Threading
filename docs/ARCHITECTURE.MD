# Threading Architecture

## Purpose

EDP-Threading owns two execution models: finite `Task<TResult>` work executed by bounded Task facilities, and persistent topology-owned Dedicated Threads controlled through `Thread<TThreadIdentity>`.

Tasks do not intrinsically own native execution contexts. Workers are internal reusable contexts. Dedicated Threads own independent persistent Platform execution contexts and never consume Task Worker capacity. A `DedicatedWorkerLease` is a separate critical-Task facility with exactly one isolated Worker; it is not a public Worker object and does not borrow from an ordinary pool.

## Platform boundary

Threading consumes `EDP-Platform` execution and synchronization mechanisms. It does not expose FreeRTOS, ESP-IDF, Arduino or operating-system vocabulary through its public execution semantics.

Logical execution priority is Low, Normal, High or Critical. Processor affinity is expressed through Platform's logical affinity abstraction.

Every managed sequential execution context owns one reusable targeted wake primitive. Dense topology-derived execution-context indices route wakes structurally; Threading does not maintain a RAM pointer registry.

## Time boundary

All monotonic deadlines and physical-duration calculations derive from the application-selected canonical `EDP-Clock` monotonic timeline. Native scheduler ticks are implementation mechanisms only. Internal wakes never restart a caller's timeout budget.

## Composition

`ThreadingTopology<...>` describes statically bounded execution resources. It derives resource counts and total managed execution-context capacity at compile time. A topology offers TaskExecution only when a general Task facility exists and DedicatedThreadExecution only when a Dedicated Thread exists.

Requirements may constrain semantic identity, minimum stack capacity, minimum Worker concurrency, logical priority and affinity. Invalid or insufficient topology choices fail at compile time.

Composition validates architecture; it is not a runtime service locator or dependency-injection container.

## Bootstrap

Initialization and infrastructure Start are separate phases. Initialization validates and realizes resources but starts no EDP-managed execution context. After the application's global initialization barrier, infrastructure contexts start in application-defined absolute order.

Start is transactional: the first Platform Start failure prevents later Starts, requests cooperative termination of already-started trampolines, joins them, destroys all initialized context resources and never commits semantic activation.

Task dispatch and Dedicated Thread semantic activation are valid only while the global Threading lifecycle is operational.
