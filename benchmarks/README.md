# End-to-End Version Comparison

`compare.py` benchmarks committed versions of Popnet without modifying simulator code.
It exports each Git revision and its exact fmt/json submodule revisions, builds both in
Release mode, runs the dev tests, generates identical workloads, and measures each
executable sequentially on one CPU. It requires Linux, Python 3.9+, Git, tar, CMake,
a C/C++ compiler, GNU time (`/usr/bin/time`), taskset, and compatible Boost libraries.
The fmt and json submodules must already be initialized locally.

From the repository root:

```sh
python3 benchmarks/compare.py \
    --original-ref 9f54d67 --dev-ref 41db200 \
    --output experiment/comparison-new \
    --boost-prefix experiment/deps/usr \
    --repetitions 5 --cpu 0 --jobs 4
```

Choose a new output directory for every experiment. Omit `--boost-prefix` for a
system installation. The runner adds a supplied prefix's `lib` directory to its
process-local `LD_LIBRARY_PATH`. The original CMake setup requests Boost.System,
so this experiment uses Boost 1.88, which still provides that component's package.
Boost 1.92 works with dev alone but is not a drop-in dependency for the original
CMake setup. Neither tested source tree receives compatibility patches.

`--original-ref` and `--dev-ref` default to `main` and `dev`. Full commit IDs,
binary and input hashes, build commands, representative compiler flags, environment
details, and the generator seed are recorded. `--resume-build` can resume an
interrupted build using the same arguments, provided measurements have not started.
`--resume-measurements` continues missing samples in an existing experiment after
checking commit IDs, executable/input hashes, configuration, and environment.
It retains completed samples and records the resumption in the metadata; use a
new directory for an independent experiment. Unrecorded attempts are kept, and
retries use separate directories so append-mode output cannot contaminate results.

## Workloads

All workloads use the same 9x9 XY network, four virtual channels, 12-flit input and
output buffers, five flits per packet, simulator seed 1, and a generous simulation
limit. Packet loss and protocol mode are disabled.

| Name | Packets | Purpose |
| --- | ---: | --- |
| bundled-random | 11,983 | The repository's existing `tests/random_trace/bench` |
| uniform | 12,000 | Random distinct endpoints, one packet every two cycles |
| hotspot | 12,000 | Random sources targeting the center, one packet every two cycles |
| single-link-burst | 4,000 | Simultaneous packets on one link; source-timing control |

The synthetic traces use generator seed `20260924`. They are simple simulator
workloads, not standardized application benchmarks.

## Measurements And Checks

Each workload/version gets one discarded warm-up and five measured runs by default.
The version that runs first alternates between repetitions. Wall time is measured
with a monotonic clock around process execution; GNU time records user/system CPU
time and maximum resident memory. Builds, workload generation, and validation are
outside the timed region. Startup, trace parsing, simulation, default debug-level
file logging, delay output, and shutdown are included. The filesystem uses its
normal cache; these are neither cold-cache nor durable-write measurements.

Every recorded run must exit successfully and produce correctly shaped, finite,
nonnegative delay records. The report checks whether the entire input completed,
source/destination counts match, input timestamps are preserved, delay outputs are
equal ignoring record order, and each version repeats byte-for-byte. Incomplete
runs remain in the data but receive no completed-work speedup claim. A faster run
with a different simulation result is not proof of a pure algorithmic speedup.

The original has a source-local timing bug that dev fixes. The simultaneous burst
avoids differing scheduled timestamps and provides a control, but its equivalence
is still checked explicitly. No conclusion about other routing modes is implied.

## Artifacts

The requested experiment report is in `../experiment/README.md`. The measured run
directory contains:

- `report.md`: generated performance and correctness tables.
- `samples.csv`: every warm-up and measured sample; no discarded timed outliers.
- `metadata.json`: exact revisions, environment, inputs, and build provenance.
- `inputs/`: all four input traces.
- `runs/`: per-run configs, simulator/console logs, delay records, and resource usage.
- `sources/`, `builds/`, and `build-*.log`: clean source exports and build evidence.

Raw debug logs occupy several gigabytes. The runner retains them for inspection
and never overwrites individual run outputs. Git ignores downloaded dependencies,
exported source/build trees, and per-run logs and delay dumps. Reports, input
traces, configs, CSV/JSON measurements, resource summaries, and compact build logs
remain trackable.
