# Architecture

Threading sits above Platform and BoundedTopology. Platform owns native execution contexts and synchronization; BoundedTopology owns dependency-minimal compact bounded-index, membership-set and intrusive-FIFO mechanics; Threading owns semantic Task/Thread lifecycle, static execution topology, scheduling policy, cooperative cancellation/stop, bounded waiting and shutdown.

A `ThreadingTopology` statically declares all managed resources. Each native execution context receives a dense topology-derived index; runtime addressing is structural rather than a heap-backed pointer registry. `StaticThreadingRuntime` owns the complete Bootstrap/runtime surface, including targeted wake topology, bindings and shutdown coordination.

Task-record and Worker availability are represented by `EDP-BoundedTopology::BoundedIndexSet`. Queued Tasks are ordered by `EDP-BoundedTopology::IntrusiveQueue`. These shared primitives carry no Threading lifecycle semantics or synchronization.

`TaskRecord::QueueOrExecutionContext` remains one compact raw scalar reused across mutually exclusive lifecycle phases. Strong bounded identities are materialized only at set/queue boundaries, preserving the existing memory optimization.
