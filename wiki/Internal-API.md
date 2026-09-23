# Internal API

Internal machinery includes capacity-derived record indices, intrusive FIFO linkage, dense context routing, per-context targeted wake state, static topology resolution, shutdown registration and in-place provider/resource storage.

These mechanisms must remain bounded by compile-time topology. Internal routing types are not consumer extension points unless explicitly surfaced by a public template contract.
