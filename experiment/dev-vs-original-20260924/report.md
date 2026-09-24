# Dev Versus Original: Benchmark Results

Measured 2026-09-24T10:10:43.854917+00:00.

- Original: `9f54d6716f2bf7474790c686ba3bc84f37cfcc27` (`main`).
- Dev: `41db2008e7b08485e9cbf5f4668f59cf9342c86e` (`dev`).
- CPU: Intel(R) Core(TM) Ultra 5 225; all simulator runs pinned to CPU 0.
- OS: Linux-7.2.6-arch2-1-x86_64-with-glibc2.44.
- Compiler: c++ (GCC) 16.2.1 20260810; cmake version 4.4.3.
- Boost: 1.88; CPU governor: powersave.
- Both committed source trees are unmodified; identical submodule revisions and Boost installation.
- Release builds, `-O3 -DNDEBUG`, no sanitizers; build commands and flags in `metadata.json`.
- 5 measured runs per workload/version after one warm-up each. Versions alternate first position each repetition; runs are sequential.
- Default Debug-level file logging, delay output, startup, parsing, simulation, and shutdown are timed.
- File writes use the normal OS cache; no fsync or cold-cache claim. Input generation, compilation, tests, and output validation are excluded.
- Wall time uses a monotonic clock; CPU time and peak RSS use GNU time. RSS is the median of per-process peaks, not an incremental allocation measurement.

## Workloads

All workloads use 9x9 XY routing, 4 VCs, 12-flit input/output buffers, 4-atom flits, link length 1000, simulator seed 1, and a 200,000-cycle limit. Protocol mode, packet loss, and trace following are disabled. Every packet has 5 flits.

| Workload | Packets | Last injection cycle | Description |
| --- | ---: | ---: | --- |
| bundled-random | 11,983 | 10001 | Bundled 9x9 random trace, 5 flits/packet |
| uniform | 12,000 | 23998 | Uniform random source/destination, one packet every 2 cycles |
| hotspot | 12,000 | 23998 | Random sources to (4,4), one packet every 2 cycles |
| single-link-burst | 4,000 | 0 | All at cycle 0, (0,0) to (0,1); timing-bug control |

Synthetic generator seed: `20260924`. The bundled input comes directly from the original commit's `tests/random_trace/bench`. Input SHA-256 values are in `metadata.json`.

## End-to-End Performance

Medians; the range in parentheses is the minimum to maximum of the measured runs. Speedup is original wall time divided by dev wall time.

| Workload | Original wall s (range) | Dev wall s (range) | Speedup | Original CPU s | Dev CPU s | Original RSS MiB | Dev RSS MiB |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| bundled-random | 4.421 (4.281-4.908) | 1.048 (1.036-1.235) | 4.22x | 4.240 | 1.020 | 21.09 | 13.55 |
| uniform | 4.595 (4.507-4.610) | 1.228 (1.218-1.236) | 3.74x | 4.530 | 1.210 | 20.20 | 13.38 |
| hotspot | 6.073 (6.055-6.159) | 1.182 (1.180-1.208) | N/A (incomplete) | 6.040 | 1.170 | 20.88 | 14.91 |
| single-link-burst | 0.525 (0.523-0.529) | 0.301 (0.299-0.303) | 1.75x | 0.510 | 0.290 | 10.03 | 12.21 |

## Output Checks

Every recorded run exits successfully and emits six-field finite nonnegative delay records. Completion and source/destination counts are checked against the input; incomplete runs are retained but excluded from completed-work speedup claims. Timestamp mismatches count missing `(integer_start_time, source, destination)` records relative to the input multiset. Average delays use the serialized delay values, which have limited precision.

| Workload | Completed original / dev | Original average delay | Dev average delay | Timestamp mismatches original / dev | Same delay records (order ignored) | Repeatable original / dev |
| --- | ---: | ---: | ---: | ---: | --- | --- |
| bundled-random | 11983 / 11983 | 854.700954 | 35.586068 | 11848 / 0 | no | yes / yes |
| uniform | 12000 / 12000 | 780.572583 | 35.473500 | 11869 / 0 | no | yes / yes |
| hotspot | 11987 / 12000 | 8709.091266 | 1391.371250 | 11226 / 0 | no | yes / yes |
| single-link-burst | 4000 / 4000 | 10012.710750 | 10012.710750 | 0 / 0 | yes | yes / yes |

## Interpretation And Limits

These are unmodified-version, end-to-end measurements, including logging and file I/O. They do not isolate routing or event-queue speed. The original source-timing bug can inject future source-local packets prematurely and alter their timestamps; dev fixes this. Stable equal-time ordering also differs. Workloads with different delay records therefore cannot establish speedup for identical simulated event histories.

The simultaneous single-link burst avoids differing scheduled timestamps and serves as a control; its output equivalence is measured above, not assumed. All samples, including warm-ups, are in `samples.csv`. Each run retains its JSON config, console log, simulator log, delay records, and GNU time output under `runs/`.

This small suite covers XY routing only. It does not establish performance for graph reconfiguration, chiplet routing, protocol traffic, or trace-following mode. Measurements are from one shared workstation; frequency scaling and other activity can affect timings. No statistical significance or cross-machine result is claimed.
