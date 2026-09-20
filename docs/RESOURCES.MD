# Deterministic Threading Resources

## Design rule

Memory optimization is architectural. Threading uses statically bounded records, Workers, stacks, queues and waiter capacity; it does not use hidden heap fallback or unbounded runtime growth in EDP-controlled execution paths.

## Task records

A Task record reuses one aligned payload region for callable storage and, after callable lifetime ends, result storage. Intrinsic lifecycle/owner/Phase control targets one byte. Free-record availability is facility-level, and queued linkage uses the smallest capacity-derived record index.

Task record capacity and Worker concurrency are independent. Terminal records retained only by ownership/waiters are not execution work.

## Execution contexts

Each Worker and Dedicated Thread owns a long-lived Platform execution context. `DedicatedWorkerLease` owns exactly one isolated Worker context.

The topology derives one dense `ExecutionContextIndex` space at compile time. Current-context/interruption resolution is structural; no default per-context pointer table is required.

Caller-supplied native control backing and stack backing remain distinct from the concrete Platform provider object's own bytes and must all be counted in deterministic resource reporting.

## Waiters and wake mechanisms

Wait registrations are bounded and target-owned. Task waits encode record index + Phase; Dedicated Thread Join encodes activation Phase; terminal shutdown needs only waiting-context registration.

Every managed execution context owns one reusable targeted wake primitive. Wake notification is mechanism only: protected Threading state is always re-evaluated after wake.

## Measurement

Exact target RAM totals are not inferred from nominal field widths. Completion validation records compiled `sizeof`/`alignof`, Platform provider-object size, caller control backing, rounded/aligned stack backing and synchronization backing for representative topologies.

Required measurements include small and representative ordinary facilities, a higher-priority control facility, one DedicatedWorkerLease, one Dedicated Thread, and a mixed topology.
