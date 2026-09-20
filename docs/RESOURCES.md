# EDP-Threading Deterministic Resources

All runtime capacities are compile-time declarations. There is no heap fallback for Task records, callable payloads, result payloads, waiter registrations, Worker topology or Dedicated Thread topology.

## Task storage

Task facilities use capacity-derived compact record indices, a bounded availability bitmap and intrusive FIFO linkage. Callable and result storage are separately bounded. Public Task ownership is binary and move-only; no reference count is maintained.

## Managed contexts

Every Worker, Dedicated Worker and Dedicated Thread contributes one dense topology context index. One reusable targeted wake primitive is owned per context. Structural routing avoids a RAM table of context pointers.

## Dedicated resources

`DedicatedWorkerLease` declares its record/callable/result capacities plus stack, priority and affinity. `DedicatedThread` declares execution properties while its application callable is supplied separately with `BindDedicatedThread<Identity>()`.

## Platform accounting

Threading distinguishes the C++ provider object, caller-supplied native control backing and stack backing. `StackCapacity<N>` is the semantic requested minimum; the concrete Platform provider may round reservation according to native alignment/granularity. Final v1 completion requires measured provider-object, control, stack, wake and Threading-intrinsic bytes for representative topologies.

## Zero-resource topology

An empty topology owns no signals or execution contexts. Empty wake/router/resolver implementations exist specifically so optional Threading Composition does not reserve an array or context registry merely because the library is present.
