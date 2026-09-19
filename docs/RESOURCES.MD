# Deterministic Threading Resources

## Design rule

Memory optimization is architectural. Threading uses statically bounded records, Workers, stacks, queues and waiter capacity; it does not use hidden heap fallback or unbounded runtime growth in EDP-controlled execution paths.

## Task records

A Task record reuses one aligned payload region for callable storage and, after callable lifetime ends, result storage. Intrinsic lifecycle/owner/Phase control targets one byte. Free-record availability is facility-level, and queued linkage uses the smallest practical bounded record index.

Task record capacity and Worker concurrency are independent.

## Execution contexts

Each Worker and Dedicated Thread owns a long-lived Platform execution context. Caller-supplied native control backing and stack backing remain distinct from the concrete Platform provider object's own bytes and must all be counted in deterministic resource reporting.

## Waiters

Wait registrations are bounded and target-owned. Managed execution contexts use reusable targeted wake resources. Wake notification is mechanism only: protected Threading state is always re-evaluated after wake.

## Measurement

Exact target RAM totals are not inferred from nominal field widths. Implementation validation records compiled sizeof/alignof, Platform provider-object size, caller control backing, stack rounding/alignment and synchronization backing for representative topologies.
