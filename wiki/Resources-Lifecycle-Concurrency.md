# Resources, Lifecycle and Concurrency

All capacities are compile-time. Task records, queues, callable/result buffers, context indices, router state and wake state are bounded. One targeted wake mechanism is retained per managed sequential context and zero-resource topologies compile corresponding machinery away.

EDP-BoundedTopology-backed storage preserves the existing Threading memory targets: one availability bit per Task record/Worker and two compact bounded indices per intrusive FIFO. No cached availability count or queue-size state is introduced.

`TaskRecord::QueueOrExecutionContext` remains one lifecycle-reused compact scalar. Queue linkage and execution-context identity are mutually exclusive, so strong identities are reconstructed at topology boundaries rather than both being retained.

Lifecycle is construction -> Initialize -> Start -> operational -> BeginShutdown -> execution quiescence -> FinalizeShutdown. Shutdown is cooperative and non-forcing. Canonical waits use EDP-Clock monotonic time. No general ISR-safety claim applies to Threading operations.
