# Deterministic Threading Resources

## Design rule

Memory optimization is architectural. Threading uses statically bounded records, Workers, stacks, queues and waiter capacity; it does not use hidden heap fallback or unbounded runtime growth in EDP-controlled execution paths.

## Task records

A Task record reuses one aligned payload region for callable storage and, after callable lifetime ends, result storage. Intrinsic lifecycle/owner/Phase control targets one byte. Free-record availability is facility-level, and queued linkage uses the smallest capacity-derived record index.

Task record capacity and Worker concurrency are independent. Terminal records retained only by ownership/waiters are not execution work. Callable and result storage are separately bounded; public Task ownership is binary and move-only rather than reference counted.

## Execution contexts

Each ordinary Worker and Dedicated Thread owns a long-lived Platform execution context. `DedicatedWorkerLease` owns exactly one isolated Worker context.

The topology derives one dense `ExecutionContextIndex` space at compile time. Current-context/interruption resolution is structural; no default per-context pointer table is required. Every managed execution context owns one reusable targeted wake primitive.

## Waiters and wake mechanisms

Wait registrations are bounded and target-owned. Task waits encode record index + Phase; Dedicated Thread Join encodes activation Phase; terminal shutdown needs only waiting-context registration.

Wake notification is mechanism only: protected Threading state is always re-evaluated after wake. The static runtime owns its bounded shutdown wait surface.

## Dedicated resources

`DedicatedWorkerLease` declares record/callable/result capacities plus stack, priority and affinity. `DedicatedThread` declares execution properties while its application callable is supplied separately with `BindDedicatedThread<Identity>()`.

## Platform accounting

Threading distinguishes the C++ provider object, caller-supplied native control backing and stack backing. `StackCapacity<N>` is the semantic requested minimum; the concrete Platform provider may round reservation according to native alignment/granularity.

Exact target RAM totals are not inferred from nominal field widths. Completion validation records compiled `sizeof`/`alignof`, Platform provider-object size, caller control backing, rounded/aligned stack backing and synchronization backing for representative topologies.

Required measurements include small and representative ordinary facilities, a higher-priority control facility, one DedicatedWorkerLease, one Dedicated Thread, and a mixed topology.

## Zero-resource topology

An empty topology owns no signals or execution contexts. Empty wake/router/resolver implementations exist specifically so optional Threading Composition does not reserve an array or context registry merely because the library is present. Its shutdown wait specialization also omits waiter-registration and mutex-provider backing.
