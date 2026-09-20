# EDP-Threading Lifecycle

## Initialization

Construction allocates/owns all statically configured backing but starts no EDP-managed execution context. `Initialize()` validates targeted wake providers and initializes Platform execution providers in declaration order. A failure destroys the failing provider and rolls back already initialized resources. Only successful completion commits the global initialization barrier.

## Infrastructure Start

`Start()` starts resources in topology declaration order. The operation is transactional: the first failure prevents later starts, requests termination of already-started trampolines, joins them and destroys initialized resources. Semantic activation is unavailable until the entire infrastructure Start commits successfully.

## Semantic execution

After operational commit:

- ordinary Tasks are dispatched through their Pool identity;
- Dedicated Worker work is dispatched through its critical Task identity;
- Dedicated Threads are accessed through Thread identity and explicitly activated;
- `Yield`, `SleepFor`, `SleepUntil`, Task waits and Thread joins remain predicate-driven and interruption-aware.

## Shutdown

`BeginShutdown()` is terminal. It changes the authoritative lifecycle first, then structurally traverses the topology. Task resources cancel queued work and request cooperative cancellation of running work; Dedicated Threads receive stop requests.

`IsExecutionQuiescent()` scans every resource. A non-cooperating user callable can therefore prevent shutdown forever by design.

After quiescence, `FinalizeShutdown()` requests infrastructure termination, joins and destroys all persistent contexts, publishes `ShutdownComplete`, then wakes shutdown waiters. Completion publication occurs once and only after teardown.
