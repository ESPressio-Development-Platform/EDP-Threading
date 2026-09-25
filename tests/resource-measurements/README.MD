# EDP-Threading Resource Measurements

This build-only harness implements the representative resource measurements required by the EDP-Threading implementation/validation matrix.

It measures six representative topologies against both concrete ESP32 execution-provider surfaces:

1. small 8-record / 1-Worker Task facility;
2. representative 16-record / 2-Worker Task facility;
3. small Critical-priority Control facility;
4. one DedicatedWorkerLease;
5. one Dedicated Thread;
6. mixed topology containing two Task facilities, one DedicatedWorkerLease and two Dedicated Threads.

The mixed scenario also links Task wait surfaces across the two facilities so the code-size measurement retains the cross-facility wait path.

Two baseline environments are built as well, one for ESP-IDF and one for Arduino. Those provide framework/toolchain text-size baselines for later code-size deltas.

## RAM categories

For each non-baseline build, target-compiled symbol sizes expose:

- total EDP-governed deterministic Threading runtime bytes;
- Threading intrinsic bytes;
- concrete execution-context provider object bytes;
- caller-supplied native execution-control backing;
- provider-rounded physical stack backing;
- synchronization/wake backing.

Synchronization/wake backing includes the topology's targeted Signal providers, resource/shutdown Mutex providers, and the single lifecycle SpinLock provider.

The category sum is compile-time checked against sizeof(StaticThreadingRuntime). No heap estimate is involved.

## Code-size evidence

The post-build reporter also captures target ELF text/data/bss sizes. The useful Threading code-size delta for each scenario is the scenario text size minus the corresponding framework baseline text size.

The measurement symbols themselves occupy BSS intentionally so their exact target-compiled byte sizes can be extracted reliably with nm. Do not interpret the harness's whole-program BSS as the Threading RAM result; use the reported total/intrinsic/provider/control/stack/sync categories instead.

## Running

From this directory:

```bash
bash ./run-resource-measurements.sh
```

No device is flashed.

The script deletes only this harness's local `.pio` directory before starting, forcing a fresh dependency resolution. It writes per-environment logs under `.measurement-logs/` and prints the compact measurement lines again at the end.

The useful output lines begin with either:

```text
EDP_THREADING_BASELINE
EDP_THREADING_MEASUREMENT
```

This is validation instrumentation only. It does not alter EDP-Threading production semantics or versioning.

## Coherent local source mode

When `EDP_THREADING_RESOURCE_LOCAL_ROOT` is supplied, the runner injects sibling production `src` paths directly and defines `EDP_THREADING_RESOURCE_COHERENT_LOCAL=1`. In that mode the measurement source includes only the ESP-IDF `ExecutionContextProvider` and `SpinLockProvider` headers it actually consumes rather than the broad ESP-IDF aggregation header.

This keeps the measurement dependency graph limited to Threading's real validation requirements and avoids pulling unrelated Memory/Security provider surfaces into a Threading resource measurement. The concrete measured provider Types are unchanged.

## C++ Language Baseline

All resource-measurement environments compile as C++20 and retain the repository's validated PIOArduino ESP-IDF platform pin so measurement comparisons remain on the same toolchain family.
