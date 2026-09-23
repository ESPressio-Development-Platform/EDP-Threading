# Resources, Lifecycle and Concurrency

All capacities are compile-time. Task records, queues, callable/result buffers, context indices, router state and wake state are bounded. One targeted wake mechanism is retained per managed sequential context and zero-resource topologies compile corresponding machinery away.

Lifecycle is construction -> Initialize -> Start -> operational -> BeginShutdown -> execution quiescence -> FinalizeShutdown. Shutdown is cooperative and non-forcing. Canonical waits use EDP-Clock monotonic time. No general ISR-safety claim applies to Threading operations.
