# Resources, Lifecycle and Concurrency

All capacities are compile-time. Task records, queues, callable/result buffers, context indices, router state and wake state are bounded. One targeted wake mechanism is retained per managed sequential context and zero-resource topologies compile corresponding machinery away.

EDP-BoundedTopology-backed storage preserves the existing Threading memory targets: one availability bit per Task record/Worker and two compact bounded indices per intrusive FIFO. No cached availability count or queue-size state is introduced.

`TaskRecord::QueueOrExecutionContext` remains one lifecycle-reused compact scalar. Queue linkage and execution-context identity are mutually exclusive, so strong identities are reconstructed at topology boundaries rather than both being retained.

Lifecycle is construction -> Initialize -> Start -> operational -> BeginShutdown -> execution quiescence -> FinalizeShutdown. Shutdown is cooperative and non-forcing. Canonical waits use EDP-Clock monotonic time. A Dedicated Thread may wait indefinitely on its existing managed-context signal; external work publication, cooperative stop and infrastructure termination all wake the same signal. A wake is advisory and the Thread rechecks every owned work source plus stop state. No general ISR-safety claim applies to Threading operations.

## Bounded-topology migration validation

Before reintegration into `main`, target-compiled retained RAM was remeasured across all fourteen existing ESP-IDF/Arduino resource environments after adopting EDP-BoundedTopology.

Every non-baseline scenario retained exactly the same `total` and `intrinsic` byte count as the validated 23 September baseline. The migration therefore adds no retained Threading RAM in the measured matrix.

Current branch validation also passes warnings-as-errors host execution, the separate Host foundation suite, and UBSan. Event-support work adds no second wake resource: `ThreadContext` retains only a runtime pointer plus one static-operation-table pointer. A keyed `OrdinaryMutexProvider` retains only its explicitly selected Platform mutex when instantiated. A `FailClosedOrdinaryMutexProvider` additionally retains one independent Platform SpinLock and one sticky Boolean failure latch; it has no waiter registry, queue, dynamic allocation or per-caller retained state.
