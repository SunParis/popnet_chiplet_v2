#!/usr/bin/env python3
"""Build two committed Popnet versions and compare end-to-end XY workloads."""

import argparse
from collections import Counter
import csv
from datetime import datetime, timezone
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import random
import re
import signal
import statistics
import subprocess
import tempfile
import threading
import time


REPO = Path(__file__).resolve().parents[1]
SUBMODULES = ("thirdparty/fmt", "thirdparty/nlohmann_json")
COMMON_CONFIG = {
    "vertices": 9,
    "dimension": 2,
    "vc_cnt": 4,
    "input_buffer": 12,
    "output_buffer": 12,
    "flit_size": 4,
    "link_length": 1000,
    "time": 200000,
    "random_seed": 1,
    "routing_algorithm": "XY",
    "report_period": 2000,
    "protocol_enable": False,
    "packet_loss": False,
    "end_with_-1": False,
}


def capture(command, cwd=REPO):
    return subprocess.check_output(command, cwd=cwd, text=True).strip()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def export_tree(repository, revision, destination):
    destination.mkdir(parents=True, exist_ok=True)
    with subprocess.Popen(
        ["git", "archive", revision], cwd=repository, stdout=subprocess.PIPE
    ) as archive:
        subprocess.run(
            ["tar", "-x", "-C", str(destination)], stdin=archive.stdout, check=True
        )
        archive.stdout.close()
        if archive.wait() != 0:
            raise RuntimeError(f"git archive failed: {repository} {revision}")


def build_version(label, revision, args):
    commit = capture(["git", "rev-parse", f"{revision}^{{commit}}"])
    source = args.output / "sources" / label
    build = args.output / "builds" / label
    export_tree(REPO, commit, source)
    submodules = {}
    for relative in SUBMODULES:
        entry = capture(["git", "ls-tree", commit, relative]).split()
        if len(entry) != 4 or entry[0] != "160000":
            raise RuntimeError(f"Expected submodule {relative} at {commit}")
        submodules[relative] = entry[2]
        export_tree(REPO / relative, entry[2], source / relative)

    configure = [
        "cmake", "-S", str(source), "-B", str(build),
        "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_POLICY_VERSION_MINIMUM=3.10",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", "-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG",
        "-DCMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG",
    ]
    if (source / "app/main.cpp").exists():
        configure += ["-DPOPNET_BUILD_TESTS=ON", "-DPOPNET_ENABLE_SANITIZERS=OFF"]
    if args.boost_prefix:
        configure.append(f"-DCMAKE_PREFIX_PATH={args.boost_prefix}")
    compile_command = ["cmake", "--build", str(build), "--parallel", str(args.jobs)]
    print(f"Building {label}: {commit[:12]}", flush=True)
    with (args.output / f"build-{label}.log").open("w") as log:
        for command in (configure, compile_command):
            subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
        if (source / "app/main.cpp").exists():
            subprocess.run(
                ["ctest", "--test-dir", str(build), "--output-on-failure"],
                stdout=log, stderr=subprocess.STDOUT, check=True,
            )

    commands = json.loads((build / "compile_commands.json").read_text())
    simulation_commands = [
        entry["command"] for entry in commands
        if "/src/" in entry["file"] or "/srcs/" in entry["file"]
    ]
    if not simulation_commands or any(
        "-O3" not in command or "-DNDEBUG" not in command
        for command in simulation_commands
    ):
        raise RuntimeError(f"Release flags missing from {label}")
    return {
        "ref": revision, "commit": commit, "submodules": submodules,
        "binary": str(build / "popnet"), "binary_sha256": sha256(build / "popnet"),
        "configure": configure, "build": compile_command,
        "representative_compile_command": simulation_commands[0],
    }


def make_workloads(output, original_source):
    directory = output / "inputs"
    directory.mkdir()
    bundled = original_source / "tests/random_trace/bench"
    workloads = [{
        "name": "bundled-random", "description": "Bundled 9x9 random trace, 5 flits/packet",
        "path": str(directory / "bundled-random.trace"),
    }]
    Path(workloads[0]["path"]).write_bytes(bundled.read_bytes())
    rng = random.Random(20260924)
    for name in ("uniform", "hotspot", "single-link-burst"):
        path = directory / f"{name}.trace"
        count = 4000 if name == "single-link-burst" else 12000
        with path.open("w") as stream:
            for index in range(count):
                if name == "single-link-burst":
                    start, source, destination = 0, 0, 1
                elif name == "hotspot":
                    start, destination = index * 2, 40
                    source = rng.randrange(80)
                    source += source >= destination
                else:
                    start, source = index * 2, rng.randrange(81)
                    destination = rng.randrange(80)
                    destination += destination >= source
                sx, sy = divmod(source, 9)
                dx, dy = divmod(destination, 9)
                stream.write(f"{start} {sx} {sy} {dx} {dy} 5\n")
        descriptions = {
            "uniform": "Uniform random source/destination, one packet every 2 cycles",
            "hotspot": "Random sources to (4,4), one packet every 2 cycles",
            "single-link-burst": "All at cycle 0, (0,0) to (0,1); timing-bug control",
        }
        workloads.append({"name": name, "description": descriptions[name], "path": str(path)})

    for workload in workloads:
        records = [line.split() for line in Path(workload["path"]).read_text().splitlines()
                   if line.strip() and line.strip() != "-1"]
        workload.update({
            "packets": len(records), "flits": sum(int(record[-1]) for record in records),
            "last_injection_cycle": max(float(record[0]) for record in records),
            "sha256": sha256(Path(workload["path"])),
        })
    return workloads


def validate_output(workload, directory):
    delay = directory / "packets.delay"
    lines = delay.read_text().splitlines()
    records = [line.split() for line in lines]
    if any(len(record) != 6 for record in records):
        raise RuntimeError(f"Malformed delay output: {directory}")
    delays = [float(record[-1]) for record in records]
    if any(not math.isfinite(value) or value < 0 for value in delays):
        raise RuntimeError(f"Nonfinite or negative packet delay: {directory}")
    expected = Counter()
    for line in Path(workload["path"]).read_text().splitlines():
        fields = line.split()
        if fields and fields[0] != "-1":
            expected[(int(float(fields[0])), *(int(value) for value in fields[1:5]))] += 1
    actual = Counter(tuple(int(value) for value in record[:5]) for record in records)
    expected_routes = Counter()
    actual_routes = Counter()
    for key, count in expected.items():
        expected_routes[key[1:]] += count
    for key, count in actual.items():
        actual_routes[key[1:]] += count
    # Inspect the log outside the timed region. It can be much larger than the trace.
    finished = None
    with (directory / "simulator.log").open() as stream:
        for line in stream:
            match = re.search(r"Total finished:\s+(\d+)\.", line)
            if match:
                finished = int(match.group(1))
    if finished != len(records):
        raise RuntimeError(f"Final report disagrees with delay record count: {directory}")
    return {
        "completed": finished,
        "average_delay": math.fsum(delays) / len(delays) if delays else 0,
        "timestamp_mismatches": sum((expected - actual).values()),
        "delay_sha256": sha256(delay),
        "sorted_delay_sha256": hashlib.sha256("\n".join(sorted(lines)).encode()).hexdigest(),
        "log_bytes": (directory / "simulator.log").stat().st_size,
        "complete": finished == workload["packets"] and actual_routes == expected_routes,
        "missing_routes": sum((expected_routes - actual_routes).values()),
        "extra_routes": sum((actual_routes - expected_routes).values()),
    }


def run_once(label, version, workload, phase, repetition, args):
    directory = args.output / "runs" / workload["name"] / f"{phase}-{repetition}-{label}"
    if directory.exists():
        directory = Path(tempfile.mkdtemp(prefix=directory.name + "-retry-", dir=directory.parent))
    else:
        directory.mkdir(parents=True)
    config = COMMON_CONFIG | {
        "trace_file": workload["path"], "log_file": str(directory / "simulator.log"),
        "delay_file": str(directory / "packets.delay"),
    }
    write_json(directory / "config.json", config)
    command = [
        "/usr/bin/time", "-f", "%U %S %M", "-o", str(directory / "resource.txt"),
        "taskset", "-c", str(args.cpu), version["binary"], "-JSON",
        str(directory / "config.json"),
    ]
    with (directory / "console.log").open("w") as console:
        start = time.perf_counter_ns()
        with subprocess.Popen(
            command, cwd=directory, stdout=console, stderr=subprocess.STDOUT,
            start_new_session=True,
        ) as process:
            def terminate():
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass

            watchdog = threading.Timer(args.timeout, terminate)
            watchdog.start()
            try:
                # A blocking wait avoids the polling latency of wait(timeout=...).
                exit_code = process.wait()
            finally:
                watchdog.cancel()
                watchdog.join()
        elapsed = (time.perf_counter_ns() - start) / 1e9
    if exit_code != 0:
        raise RuntimeError(f"Exit {exit_code}: see {directory / 'console.log'}")
    user, system, rss = (directory / "resource.txt").read_text().split()
    row = {
        "workload": workload["name"], "version": label, "phase": phase,
        "repetition": repetition, "wall_seconds": elapsed,
        "user_seconds": float(user), "system_seconds": float(system),
        "peak_rss_kib": int(rss), **validate_output(workload, directory),
    }
    write_json(directory / "measurement.json", row)
    print(f"{workload['name']:18s} {label:8s} {phase} {repetition}: "
          f"{elapsed:.3f}s, {int(rss) / 1024:.2f} MiB, "
          f"{row['completed']}/{workload['packets']} packets", flush=True)
    return row


def make_report(metadata, workloads, rows, output):
    summaries = {}
    for workload in workloads:
        for label in ("original", "dev"):
            samples = [row for row in rows if row["workload"] == workload["name"]
                       and row["version"] == label and row["phase"] == "measured"]
            walls = [row["wall_seconds"] for row in samples]
            summaries[(workload["name"], label)] = {
                "wall": statistics.median(walls), "min": min(walls), "max": max(walls),
                "rss": statistics.median(row["peak_rss_kib"] for row in samples) / 1024,
                "cpu": statistics.median(row["user_seconds"] + row["system_seconds"]
                                         for row in samples),
                "sample": samples[0],
                "complete": all(row["complete"] for row in samples),
                "repeatable": len({row["delay_sha256"] for row in rows
                                   if row["workload"] == workload["name"]
                                   and row["version"] == label}) == 1,
            }
    lines = [
        "# Dev Versus Original: Benchmark Results", "",
        f"Measured {metadata['started_utc']}.", "",
        f"- Original: `{metadata['versions']['original']['commit']}` "
        f"(`{metadata['versions']['original']['ref']}`).",
        f"- Dev: `{metadata['versions']['dev']['commit']}` "
        f"(`{metadata['versions']['dev']['ref']}`).",
        f"- CPU: {metadata['cpu_model']}; all simulator runs pinned to CPU {metadata['cpu']}.",
        f"- OS: {metadata['platform']}.",
        f"- Compiler: {metadata['compiler']}; {metadata['cmake']}.",
        f"- Boost: {metadata['boost_version']}; CPU governor: {metadata['cpu_governor']}.",
        "- Both committed source trees are unmodified; identical submodule revisions and Boost installation.",
        "- Release builds, `-O3 -DNDEBUG`, no sanitizers; build commands and flags in `metadata.json`.",
        f"- {metadata['repetitions']} measured runs per workload/version after one warm-up each. "
        "Versions alternate first position each repetition; runs are sequential.",
        "- Default Debug-level file logging, delay output, startup, parsing, simulation, and shutdown are timed.",
        "- File writes use the normal OS cache; no fsync or cold-cache claim. Input generation, "
        "compilation, tests, and output validation are excluded.",
        "- Wall time uses a monotonic clock; CPU time and peak RSS use GNU time. "
        "RSS is the median of per-process peaks, not an incremental allocation measurement.",
        "", "## Workloads", "",
        "All workloads use 9x9 XY routing, 4 VCs, 12-flit input/output buffers, "
        "4-atom flits, link length 1000, simulator seed 1, and a 200,000-cycle limit. "
        "Protocol mode, packet loss, and trace following are disabled. Every packet has 5 flits.",
        "", "| Workload | Packets | Last injection cycle | Description |",
        "| --- | ---: | ---: | --- |",
    ]
    for workload in workloads:
        lines.append(f"| {workload['name']} | {workload['packets']:,} | "
                     f"{workload['last_injection_cycle']:g} | {workload['description']} |")
    lines += [
        "", "Synthetic generator seed: `20260924`. The bundled input comes directly from "
        "the original commit's `tests/random_trace/bench`. Input SHA-256 values are in `metadata.json`.",
        "", "## End-to-End Performance", "",
        "Medians; the range in parentheses is the minimum to maximum of the measured runs. "
        "Speedup is original wall time divided by dev wall time.", "",
        "| Workload | Original wall s (range) | Dev wall s (range) | Speedup | "
        "Original CPU s | Dev CPU s | Original RSS MiB | Dev RSS MiB |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for workload in workloads:
        a = summaries[(workload["name"], "original")]
        b = summaries[(workload["name"], "dev")]
        speedup = f"{a['wall'] / b['wall']:.2f}x" if a["complete"] and b["complete"] else "N/A (incomplete)"
        lines.append(f"| {workload['name']} | {a['wall']:.3f} ({a['min']:.3f}-{a['max']:.3f}) | "
                     f"{b['wall']:.3f} ({b['min']:.3f}-{b['max']:.3f}) | "
                     f"{speedup} | {a['cpu']:.3f} | {b['cpu']:.3f} | "
                     f"{a['rss']:.2f} | {b['rss']:.2f} |")
    lines += [
        "", "## Output Checks", "",
        "Every recorded run exits successfully and emits six-field finite nonnegative delay records. "
        "Completion and source/destination counts are checked against the input; incomplete runs "
        "are retained but excluded from completed-work speedup claims. "
        "Timestamp mismatches count missing `(integer_start_time, source, destination)` "
        "records relative to the input multiset. Average delays use the serialized delay values, "
        "which have limited precision.", "",
        "| Workload | Completed original / dev | Original average delay | Dev average delay | "
        "Timestamp mismatches original / dev | Same delay records (order ignored) | "
        "Repeatable original / dev |",
        "| --- | ---: | ---: | ---: | ---: | --- | --- |",
    ]
    for workload in workloads:
        a = summaries[(workload["name"], "original")]
        b = summaries[(workload["name"], "dev")]
        same = a["sample"]["sorted_delay_sha256"] == b["sample"]["sorted_delay_sha256"]
        lines.append(f"| {workload['name']} | {a['sample']['completed']} / {b['sample']['completed']} | "
                     f"{a['sample']['average_delay']:.6f} | "
                     f"{b['sample']['average_delay']:.6f} | "
                     f"{a['sample']['timestamp_mismatches']} / {b['sample']['timestamp_mismatches']} | "
                     f"{'yes' if same else 'no'} | "
                     f"{'yes' if a['repeatable'] else 'no'} / {'yes' if b['repeatable'] else 'no'} |")
    lines += [
        "", "## Interpretation And Limits", "",
        "These are unmodified-version, end-to-end measurements, including logging and file I/O. "
        "They do not isolate routing or event-queue speed. The original source-timing bug can "
        "inject future source-local packets prematurely and alter their timestamps; dev fixes "
        "this. Stable equal-time ordering also differs. Workloads with different delay records "
        "therefore cannot establish speedup for identical simulated event histories.", "",
        "The simultaneous single-link burst avoids differing scheduled timestamps and serves "
        "as a control; its output equivalence is measured above, not assumed. "
        "All samples, including warm-ups, are in `samples.csv`. Each run retains its JSON config, "
        "console log, simulator log, delay records, and GNU time output under `runs/`.", "",
        "This small suite covers XY routing only. It does not establish performance for graph "
        "reconfiguration, chiplet routing, protocol traffic, or trace-following mode. "
        "Measurements are from one shared workstation; frequency scaling and other activity "
        "can affect timings. No statistical significance or cross-machine result is claimed.", "",
    ]
    (output / "report.md").write_text("\n".join(lines))


def resume_measurements(args, environment):
    metadata = json.loads((args.output / "metadata.json").read_text())
    for key in ("cpu", "repetitions", "common_config", "platform", "compiler", "boost_prefix",
                "boost_version", "ld_library_path", "cpu_governor"):
        if metadata[key] != environment[key]:
            raise RuntimeError(f"Cannot resume with a different {key}")
    for label, ref in (("original", args.original_ref), ("dev", args.dev_ref)):
        version = metadata["versions"][label]
        if capture(["git", "rev-parse", f"{ref}^{{commit}}"] ) != version["commit"]:
            raise RuntimeError(f"Cannot resume with a different {label} revision")
        if sha256(Path(version["binary"])) != version["binary_sha256"]:
            raise RuntimeError(f"Binary changed: {label}")
    for workload in metadata["workloads"]:
        if sha256(Path(workload["path"])) != workload["sha256"]:
            raise RuntimeError(f"Input changed: {workload['name']}")
    with (args.output / "samples.csv").open(newline="") as stream:
        rows = list(csv.DictReader(stream))
    for row in rows:
        for key in ("wall_seconds", "user_seconds", "system_seconds", "average_delay"):
            row[key] = float(row[key])
        for key in ("repetition", "peak_rss_kib", "completed", "timestamp_mismatches", "log_bytes"):
            row[key] = int(row[key])
        # Earlier runs stopped on incomplete output, so recorded rows already passed these checks.
        row["complete"] = row.get("complete", "True") == "True"
        row["missing_routes"] = int(row.get("missing_routes", 0))
        row["extra_routes"] = int(row.get("extra_routes", 0))
    metadata.setdefault("resumptions", []).append({
        "utc": environment["started_utc"], "runner_sha256": environment["runner_sha256"],
        "retained_samples": len(rows),
    })
    return metadata, rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original-ref", default="main")
    parser.add_argument("--dev-ref", default="dev")
    parser.add_argument("--output", type=Path, required=True, help="New artifact directory")
    parser.add_argument("--resume-build", action="store_true",
                        help="Reuse an interrupted build directory before measurements start")
    parser.add_argument("--resume-measurements", action="store_true",
                        help="Continue missing samples after checking binaries and inputs")
    parser.add_argument("--boost-prefix", type=Path)
    parser.add_argument("--repetitions", type=int, default=5)
    parser.add_argument("--cpu", type=int, default=min(os.sched_getaffinity(0)))
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--timeout", type=float, default=120)
    args = parser.parse_args()
    if args.repetitions < 1 or args.jobs < 1 or args.timeout <= 0:
        parser.error("repetitions, jobs, and timeout must be positive")
    if args.cpu not in os.sched_getaffinity(0):
        parser.error("selected CPU is outside the current affinity mask")
    args.output = args.output.resolve()
    if args.boost_prefix:
        args.boost_prefix = args.boost_prefix.resolve()
        library_path = str(args.boost_prefix / "lib")
        if os.environ.get("LD_LIBRARY_PATH"):
            library_path += ":" + os.environ["LD_LIBRARY_PATH"]
        os.environ["LD_LIBRARY_PATH"] = library_path
    os.environ["LC_ALL"] = "C"
    if args.resume_build and any((args.output / item).exists()
                                 for item in ("inputs", "runs", "samples.csv")):
        parser.error("measurements already started; select a new output directory")
    args.output.mkdir(parents=True, exist_ok=args.resume_build or args.resume_measurements)
    cpu_info = json.loads(capture(["lscpu", "-J"]))["lscpu"]
    boost_header = (args.boost_prefix or Path("/usr")) / "include/boost/version.hpp"
    boost_version = "see CMakeCache.txt"
    if boost_header.exists():
        boost_version = re.search(r'#define BOOST_LIB_VERSION "([^"]+)"',
                                  boost_header.read_text()).group(1).replace("_", ".")
    governor = Path(f"/sys/devices/system/cpu/cpu{args.cpu}/cpufreq/scaling_governor")
    metadata = {
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "platform": platform.platform(), "python": platform.python_version(),
        "compiler": capture([os.environ.get("CXX", "c++"), "--version"]).splitlines()[0],
        "cmake": capture(["cmake", "--version"]).splitlines()[0],
        "cpu_model": next(item["data"] for item in cpu_info if item["field"] == "Model name:"),
        "cpu": args.cpu, "repetitions": args.repetitions, "common_config": COMMON_CONFIG,
        "boost_prefix": str(args.boost_prefix) if args.boost_prefix else None,
        "boost_version": boost_version,
        "ld_library_path": os.environ.get("LD_LIBRARY_PATH", ""),
        "cpu_governor": governor.read_text().strip() if governor.exists() else "unavailable",
        "runner_sha256": sha256(Path(__file__)),
    }
    rows = []
    if args.resume_measurements:
        metadata, rows = resume_measurements(args, metadata)
    else:
        versions = {}
        for label, revision in (("original", args.original_ref), ("dev", args.dev_ref)):
            versions[label] = build_version(label, revision, args)
        if versions["original"]["submodules"] != versions["dev"]["submodules"]:
            raise RuntimeError("Dependency revisions differ; cannot claim identical dependencies")
        metadata["versions"] = versions
        metadata["workloads"] = make_workloads(args.output, args.output / "sources/original")
    versions = metadata["versions"]
    workloads = metadata["workloads"]
    write_json(args.output / "metadata.json", metadata)
    finished = {(row["workload"], row["version"], row["phase"], row["repetition"]) for row in rows}
    with (args.output / "samples.csv").open("w", newline="") as stream:
        writer = None
        if rows:
            writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
            stream.flush()
        for workload in workloads:
            for index in range(args.repetitions + 1):
                phase = "warmup" if index == 0 else "measured"
                order = ("original", "dev") if index % 2 == 0 else ("dev", "original")
                for label in order:
                    if (workload["name"], label, phase, index) in finished:
                        continue
                    row = run_once(label, versions[label], workload, phase, index, args)
                    if writer is None:
                        writer = csv.DictWriter(stream, fieldnames=list(row))
                        writer.writeheader()
                    writer.writerow(row)
                    stream.flush()
                    rows.append(row)
    make_report(metadata, workloads, rows, args.output)
    print(f"Report: {args.output / 'report.md'}", flush=True)


if __name__ == "__main__":
    main()
