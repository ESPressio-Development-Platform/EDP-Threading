# Internal API

Internal machinery includes shared capacity-derived bounded-index representation, one-bit availability sets, intrusive FIFO linkage, dense context routing, per-context targeted wake state, static topology resolution, shutdown registration and in-place provider/resource storage.

`detail/TopologyIndex.hpp` is the Threading-internal adapter between EDP-BoundedTopology's strong bounded identities and Threading's raw compact scalar fields. It adds no runtime state and preserves Threading's existing 32-bit maximum index contract.

These mechanisms must remain bounded by compile-time topology. Internal routing types are not consumer extension points unless explicitly surfaced by a public template contract.
