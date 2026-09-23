# Deterministic Threading Resources

## Design rule

Memory optimization is architectural. Threading uses statically bounded records, Workers, stacks, queues and waiter capacity; it does not use hidden heap fallback or unbounded runtime growth in EDP-controlled execution paths.

## Task records

A Task record reuses one aligned payload region for callable storage and, after callable lifetime ends, result storage. Intrinsic lifecycle/owner/Phase control remains exactly one byte. Its mutation and observation are serialized through the facility mutex; cooperative cancellation observed from the running callable re-enters that same bounded synchronization boundary through its invocation-local `TaskContext`. This preserves the one-byte per-record target without requiring byte-wide lock-free compare/exchange on platforms whose native atomic width is larger. Free-record availability is facility-level, and queued linkage uses the smallest capacity-derived record index.

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

## Validated ESP32 resource measurements — 2026-09-23

The following measurements were produced on the `feature/composition_framework_enhancements` source graph using the coherent local-source resource runner.

Toolchain baseline:

- PIOArduino Core 6.2.0;
- PIOArduino Espressif32 platform commit `d41d8383706dc146fd91a0e1d3c59a915a4d493e`;
- Espressif32 platform 55.3.312;
- ESP-IDF 5.5.5;
- Arduino-ESP32 3.3.12;
- Xtensa ESP32 GCC 14.2.0.

The `total` column is the complete statically retained Threading runtime storage for the measured topology. It is decomposed into Threading-intrinsic storage, concrete execution-provider objects, caller-owned native control backing, provider-rounded stack backing, and synchronization/wake provider storage.

### ESP-IDF

| Scenario | Total | Intrinsic | Provider objects | Control backing | Stack backing | Sync/wake | Text | Data | BSS |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| baseline | — | — | — | — | — | — | 115041 | 47208 | 2281 |
| small 8×1 | 5568 | 668 | 192 | 340 | 4096 | 272 | 128569 | 47336 | 7857 |
| representative 16×2 | 10880 | 1264 | 384 | 680 | 8192 | 360 | 129249 | 47352 | 13169 |
| control high | 3104 | 252 | 192 | 340 | 2048 | 272 | 128453 | 47336 | 5393 |
| dedicated worker | 3064 | 212 | 192 | 340 | 2048 | 272 | 128373 | 47336 | 5353 |
| dedicated thread | 4972 | 72 | 192 | 340 | 4096 | 272 | 121913 | 47320 | 7265 |
| mixed | 19152 | 1180 | 960 | 1700 | 14336 | 976 | 150845 | 47544 | 21441 |

### Arduino

| Scenario | Total | Intrinsic | Provider objects | Control backing | Stack backing | Sync/wake | Text | Data | BSS |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| baseline | — | — | — | — | — | — | 191399 | 71073 | 5420 |
| small 8×1 | 5624 | 672 | 208 | 352 | 4096 | 296 | 198867 | 71225 | 11052 |
| representative 16×2 | 10968 | 1264 | 416 | 704 | 8192 | 392 | 200151 | 71269 | 16396 |
| control high | 3160 | 256 | 208 | 352 | 2048 | 296 | 199771 | 71245 | 8588 |
| dedicated worker | 3120 | 216 | 208 | 352 | 2048 | 296 | 199031 | 71221 | 8548 |
| dedicated thread | 5024 | 72 | 208 | 352 | 4096 | 296 | 195955 | 71189 | 10452 |
| mixed | 19392 | 1192 | 1040 | 1760 | 14336 | 1064 | 212047 | 71373 | 24820 |

All fourteen measurement environments built successfully.

These results are build-time measurements only; no hardware flashing is required by the resource runner. The baseline rows intentionally contain no retained Threading runtime storage and provide the framework image reference for code/data/BSS comparison.

