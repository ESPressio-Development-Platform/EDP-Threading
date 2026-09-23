# Architecture

Threading sits above Platform. Platform owns native execution contexts and synchronization; Threading owns semantic Task/Thread lifecycle, static topology, scheduling policy, cooperative cancellation/stop, bounded waiting and shutdown.

A `ThreadingTopology` statically declares all managed resources. Each native execution context receives a dense topology-derived index; runtime addressing is structural rather than a heap-backed pointer registry. `StaticThreadingRuntime` owns the complete Bootstrap/runtime surface, including targeted wake topology, bindings and shutdown coordination.
