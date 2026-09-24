# Experiment: Dev Versus Original Popnet

Date: 2026-09-24 (Asia/Taipei).

`dev` is faster on the three workloads that both versions complete. The strongest
comparison is the single-link burst: its delay files are byte-identical, and dev
takes 0.301 seconds instead of 0.525 seconds, a **1.75x speedup**. On the bundled
random and synthetic uniform traces, dev is **4.22x** and **3.74x** faster, but the
original's incorrect source timing changes the simulated results. Those are
end-to-end version comparisons, not measurements of identical event histories.

The hotspot exposes a correctness difference: the original completes only 11,987
of 12,000 packets by the simulation limit, while dev completes all 12,000. Its
timing is retained below without a completed-work speedup claim. Memory usage is
also workload-dependent: dev uses less peak memory on the larger traces but
21.8% more on the small burst control.

## Versions And Environment

| Item | Setting |
| --- | --- |
| Original | `main` / `origin/main`, `9f54d6716f2bf7474790c686ba3bc84f37cfcc27` |
| Dev | `dev` / `origin/dev`, `41db2008e7b08485e9cbf5f4668f59cf9342c86e` |
| CPU | Intel Core Ultra 5 225; simulator pinned to logical CPU 0 |
| OS | Arch Linux, kernel `7.2.6-arch2-1`, glibc 2.44 |
| Compiler | GCC 16.2.1, 20260810 |
| Build tools | CMake 4.4.3; Python 3.14.7 benchmark driver |
| Dependencies | Boost 1.88.0 and ICU 76.1, shared local installation |
| Optimization | Release, `-O3 -DNDEBUG`, sanitizers disabled |
| Frequency policy | Existing `powersave` governor; not changed for the experiment |

Both simulators were built from clean Git exports, including the exact fmt and
nlohmann/json submodule commits. No simulator source or behavior was patched.
The same compiler and optimization settings were used. Existing project-specific
flags remain, including the original's `-g` and GNU C++ extensions. Full commands,
compiler flags, executable hashes, dependency revisions, and input hashes are in
[metadata.json](dev-vs-original-20260924/metadata.json).

The rebuilt dev unit and integration CTest targets both passed before measurement.

## Benchmark Selection

The repository already contains a usable random trace at
`tests/random_trace/bench`; it is byte-identical in the two commits. Three small
synthetic workloads add coverage of distributed traffic, congestion, and a
controlled identical-output case. These are simulator workloads, not standardized
application benchmarks.

| Workload | Packets | Flits | Injection pattern | Purpose |
| --- | ---: | ---: | --- | --- |
| Bundled random | 11,983 | 59,915 | Existing trace, last injection at cycle 10,001 | Representative repository workload |
| Uniform | 12,000 | 60,000 | Random distinct endpoints; one packet every 2 cycles | Distributed traffic |
| Hotspot | 12,000 | 60,000 | Random sources to router (4,4); one packet every 2 cycles | Concentrated traffic and completion check |
| Single-link burst | 4,000 | 20,000 | All at cycle 0, from (0,0) to (0,1) | Avoid different source timestamps; compare identical outputs |

All cases use a 9x9 XY network, four virtual channels, input and output buffers of
12 flits, five flits per packet, four atoms per flit, link length 1000, and
simulation seed 1. The simulation limit is 200,000 cycles. Protocol mode, packet
loss, and trace-following are disabled. Synthetic generation uses seed `20260924`.
The full traces are retained in [inputs/](dev-vs-original-20260924/inputs/).

## Measurement Method

Each workload/version has one recorded warm-up and five timed samples: 48 recorded
runs total, of which 40 contribute to the reported medians. Runs are sequential,
and the version running first alternates between repetitions. No measured
outliers were removed. The first two workloads' measurements were preserved when
the harness was extended to report incomplete hotspot runs; the resumption and
runner hashes are recorded in the metadata. One additional initial hotspot
warm-up, which triggered that validation failure, is retained in
`runs/hotspot/warmup-0-original/` but is not a timed statistical sample.

Wall time covers process startup, parsing, simulation, power reporting, default
debug-level logging to a file, delay output, and shutdown. It is measured using a
monotonic clock. GNU time measures user/system CPU time and per-process maximum
resident memory. Input generation, compilation, tests, and output validation are
outside the timed region. Every invocation uses fresh output files because the
simulators append delay records.

Files use the normal filesystem cache. These are not cold-cache or fsync-based
durable-write measurements. This is a shared workstation with frequency scaling;
CPU pinning does not eliminate competing workloads. Minimum-to-maximum ranges
are reported to expose timing variation.

## Runtime Results

Times are seconds. Speedup is original median wall time divided by dev median
wall time. CPU time is the median of each run's user plus system time.

| Workload | Original wall median (range) | Dev wall median (range) | Speedup | Original CPU | Dev CPU |
| --- | ---: | ---: | ---: | ---: | ---: |
| Bundled random | 4.421 (4.281-4.908) | 1.048 (1.036-1.235) | 4.22x | 4.240 | 1.020 |
| Uniform | 4.595 (4.507-4.610) | 1.228 (1.218-1.236) | 3.74x | 4.530 | 1.210 |
| Hotspot | 6.073 (6.055-6.159) | 1.182 (1.180-1.208) | Not comparable: original incomplete | 6.040 | 1.170 |
| Single-link burst | 0.525 (0.523-0.529) | 0.301 (0.299-0.303) | 1.75x | 0.510 | 0.290 |

The completed workloads reduce median wall time by 76.3%, 73.3%, and 42.7%,
respectively. Default logging and file I/O are part of these gains. For example,
median system CPU time on bundled random drops from 1.67 seconds to 0.04 seconds,
while median user CPU time drops from 2.61 seconds to 0.98 seconds. These
observations do not isolate any individual optimization's contribution.

## Memory Results

Memory is the median of per-process peak RSS measurements, in MiB. It includes
the whole executable and its loaded libraries, not just simulator allocations.

| Workload | Original peak RSS | Dev peak RSS | Dev versus original |
| --- | ---: | ---: | --- |
| Bundled random | 21.09 | 13.55 | 35.8% lower |
| Uniform | 20.20 | 13.38 | 33.8% lower |
| Hotspot | 20.88 | 14.91 | 28.6% lower; completion differs |
| Single-link burst | 10.03 | 12.21 | 21.8% higher |

There is no universal memory improvement. Dev's additional fixed storage is a
possible explanation for the burst result, but this experiment does not include
allocation profiling to establish the cause.

## Correctness And Comparability

Every recorded process exits successfully, reports a completed count matching
its delay-file line count, and emits six-field finite, nonnegative delay records.
The harness also compares completed source/destination counts to the inputs and
checks timestamp preservation. Completion is checked independently of exit code.

| Workload | Completed original / dev | Original average delay | Dev average delay | Equal delay records |
| --- | ---: | ---: | ---: | --- |
| Bundled random | 11,983 / 11,983 | 854.700954 | 35.586068 | No |
| Uniform | 12,000 / 12,000 | 780.572583 | 35.473500 | No |
| Hotspot | 11,987 / 12,000 | 8709.091266 | 1391.371250 | No |
| Single-link burst | 4,000 / 4,000 | 10012.710750 | 10012.710750 | Yes, byte-identical |

Average delays above are calculated from serialized delay records, which have
limited numeric precision; hotspot averages include only completed packets.
Each version's delay file is byte-identical across its own recorded repeats for
every workload. The complete raw measurements are in
[samples.csv](dev-vs-original-20260924/samples.csv).

The original reads the global next trace time while injecting source-local
packets. Dev fixes that bug, so the original and dev differ on scheduled traffic.
The original has 11,848 unmatched input timestamp/address records on bundled
random and 11,869 on uniform; dev has zero. The hotspot has 11,226 unmatched
records in the original, including missing completions; dev again has zero.
Stable equal-time event ordering also differs between versions. Consequently,
the random/uniform speedups describe the delivered executables on the same input,
not identical simulated event histories.

The original hotspot log continues processing router cycles through 200,001 with
13 packets unresolved. Dev completes the full workload. This establishes failure
to drain within the configured limit, not the precise cause of those unresolved
packets or proof they could never finish with another limit.

The single-link burst provides the cleanest performance evidence because both
versions finish all 4,000 packets with byte-identical delay records. The claim
remains end-to-end: power reports and internal event histories are not asserted
to be identical merely because delay output matches.

This suite covers XY routing only. It makes no performance claim for graph
reconfiguration, chiplet routing, protocol traffic, or growing traces. No aggregate
speedup is computed across incomparable workloads, and no statistical significance
or cross-machine generalization is claimed.

## Reproduction And Artifacts

From the repository root, reuse the prepared local dependencies and choose a new
output directory:

```sh
python3 benchmarks/compare.py \
    --original-ref 9f54d6716f2bf7474790c686ba3bc84f37cfcc27 \
    --dev-ref 41db2008e7b08485e9cbf5f4668f59cf9342c86e \
    --boost-prefix experiment/deps/usr \
    --output experiment/comparison-repeat \
    --repetitions 5 --cpu 0 --jobs 4
```

The runner and its documentation are in [benchmarks/](../benchmarks/). The original
CMake setup needs a Boost release that still exposes the Boost.System package;
this experiment uses Boost 1.88.0 for both versions. No system packages were
installed or replaced. These Arch packages were downloaded and extracted locally:

| Package | Archive URL | SHA-256 |
| --- | --- | --- |
| Boost headers | [boost-1.88.0-3](https://archive.archlinux.org/packages/b/boost/boost-1.88.0-3-x86_64.pkg.tar.zst) | `7859d62aaf8047c3c3005b144c9a4ebbdeedeaf633bbaa2e9aaf628583770f06` |
| Boost libraries | [boost-libs-1.88.0-3](https://archive.archlinux.org/packages/b/boost-libs/boost-libs-1.88.0-3-x86_64.pkg.tar.zst) | `05e4b82e9c548d2a33ac8e12652b53950a1a26e8a641af41fc4a0d9b28bf4167` |
| ICU runtime | [icu-76.1-1](https://archive.archlinux.org/packages/i/icu/icu-76.1-1-x86_64.pkg.tar.zst) | `8e0745f79b330395e04cb6c2119dd901b0adb35f031ab5b4240bb7e72e688157` |

Retained artifacts:

- [Generated detailed report](dev-vs-original-20260924/report.md).
- [All recorded measurements](dev-vs-original-20260924/samples.csv).
- [Environment, build, and input metadata](dev-vs-original-20260924/metadata.json).
- [Per-run JSON configs, logs, delays, and resource measurements](dev-vs-original-20260924/runs/).
- [Original build log](dev-vs-original-20260924/build-original.log) and [dev build/test log](dev-vs-original-20260924/build-dev.log).

The experiment directory occupies approximately 5 GiB, mostly retained debug
logs. Git ignores local dependencies, exported source/build trees, and per-run
logs and delay dumps. Reports, input traces, configs, compact measurements, and
build logs remain trackable. No artifacts have been staged or committed.
