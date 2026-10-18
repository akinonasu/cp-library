#!/usr/bin/env python3
"""One frozen, exact-source FPS comparison. No tuning, retries, caches, or artifacts."""

import argparse
import fcntl
import hashlib
import json
import math
import os
from pathlib import Path
import re
import signal
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[3]
BASE = "1b8607587c164b013386423ada2298e05a9e4028"
REF = "refs/heads/codex/fps-scalar-return-20261018"
SOURCE = "test/benchmark/fps_scalar_return/"
HEADER = "fps/fps.hpp"
DEPENDENCIES = ("components/fps_operations.hpp", "components/ntt.hpp", HEADER, "math/mint.hpp")
LIBRARIES = ("components", "convolution", "fps", "data_structure", "graph", "math", "string")
FROZEN = {
    "api.hpp": "0e78bf2a9c0e3af4a305b01e7520abc7f8284d5e673af80f2e022c0e7215bf97",
    "api.cpp": "4638d224c3da30e0200b0183f2599bda90a266b80892962ef8a7cb64c0d327d2",
    "bench.cpp": "009c72ddbb22b072c6459cea718d35309ef60fbded1aff65ee9e69457fb3386b",
}
CASES = {"add": (32, 300000), "sub": (262144, 128),
         "mul": (65536, 512), "div": (4096, 4096)}
CHECKSUMS = {
    "add": ("13886490060049436654", "13882659635403116537", "13082089982011685829"),
    "sub": ("11207330850518936369", "11130598958178708230", "4508857342948613413"),
    "mul": ("4568921047891222776", "7306674426695528431", "11662797323877304613"),
    "div": ("1482039196620619456", "4719237234168918346", "16645246113738105637"),
}
PAIRS = ("AB", "BA", "AB", "BA")
COMMON = ["-O2", "-std=gnu++23", "-ftrivial-auto-var-init=zero", "-fmodules",
          "-DONLINE_JUDGE", "-DATCODER", "-fno-lto", "-Wall", "-Wextra", "-Werror",
          "-Wno-error=free-nonheap-object"]
MODES = {
    "generic": ["-march=x86-64", "-mtune=generic", "-mno-avx", "-mno-avx2",
                "-mno-avx512f", "-mno-bmi", "-mno-bmi2"],
    "native": ["-march=native"],
}
MAX_COMMANDS, MAX_LOG_BYTES, COMMAND_SECONDS, JOB_SECONDS = 96, 1024 * 1024, 120, 7 * 60
LOG_BYTES = 0


def require(condition, message):
    if not condition:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def emit(event, **values):
    global LOG_BYTES
    line = json.dumps({"event": event, **values}, sort_keys=True, ensure_ascii=True, allow_nan=False)
    LOG_BYTES += len(line) + 1
    require(LOG_BYTES <= MAX_LOG_BYTES, "Comparison log budget exceeded")
    print(line, flush=True)


def unique(pairs):
    result = {}
    for key, value in pairs:
        require(key not in result, "Duplicate JSON field")
        result[key] = value
    return result


def validate_row(raw, operation):
    require(operation in CASES, "Unknown frozen case")
    row = json.loads(raw, object_pairs_hook=unique)
    require(isinstance(row, dict) and set(row) == {
        "type", "operation", "n", "repeats", "total_ns", "ns_per_call",
        "source_checksum", "result_checksum", "aggregate_checksum"}, "Unexpected benchmark JSON schema")
    require(row["type"] == "public" and row["operation"] == operation,
            "Only the fixed public fps_mint case is allowed")
    require(type(row["n"]) is int and type(row["repeats"]) is int
            and (row["n"], row["repeats"]) == CASES[operation], "Frozen size/repeats changed")
    for key in ("total_ns", "ns_per_call"):
        require(type(row[key]) in (int, float) and math.isfinite(row[key]) and row[key] > 0,
                "Invalid API timing")
    require(math.isclose(row["ns_per_call"], row["total_ns"] / row["repeats"], rel_tol=1e-5),
            "Inconsistent timing beyond frozen stream rounding")
    require(tuple(row[key] for key in ("source_checksum", "result_checksum", "aggregate_checksum"))
            == CHECKSUMS[operation], "Frozen input/output/aggregate checksum changed")
    return row


def validate_context(base, candidate, context, event):
    require(base == BASE, "Only the reviewed exact main baseline is allowed")
    require(isinstance(candidate, str) and re.fullmatch("[0-9a-f]{40}", candidate),
            "Exact candidate commit ID required")
    require(context.get("GITHUB_EVENT_NAME") == "push"
            and context.get("GITHUB_REPOSITORY") == "qandori/cp-library"
            and context.get("GITHUB_REF") == REF and context.get("GITHUB_SHA") == candidate
            and context.get("GITHUB_RUN_ATTEMPT") == "1", "Exact branch push attempt 1 required; no retries")
    require(isinstance(event, dict) and event.get("deleted") is False and event.get("ref") == REF
            and event.get("after") == candidate and isinstance(event.get("repository"), dict)
            and event["repository"].get("full_name") == "qandori/cp-library", "Push payload mismatch")


def validate_sources(baseline, candidate):
    require(set(baseline) == set(candidate) == set(DEPENDENCIES), "Unexpected snapshot dependencies")
    require(sha(baseline[HEADER]) == "49aecaa66152548f54c36e40eb21af16297f125534f241b3594520ca26fcfc40",
            "Baseline production header changed")
    expected = baseline[HEADER]
    for operation in "+-*/":
        old = f"\tF operator{operation}(T x) const {{ return F(*this) {operation}= x; }}\n".encode()
        new = (f"\tF operator{operation}(T x) const {{\n\t\tF r(*this);\n"
               f"\t\tr {operation}= x;\n\t\treturn r;\n\t}}\n").encode()
        require(expected.count(old) == 1, "Scalar baseline wrapper changed")
        expected = expected.replace(old, new)
    require(candidate[HEADER] == expected
            and sha(candidate[HEADER]) == "b29e2144d453eafa17e296ca77c603bfb8f77b04b02274bfb4d4a75d93263a34",
            "Candidate must change exactly the four reviewed scalar wrappers")
    require(all(baseline[name] == candidate[name] for name in DEPENDENCIES if name != HEADER),
            "Shared production dependency differs")


def macro_proof(raw, mode):
    require(mode in MODES, "Unknown compile mode")
    macros = dict(re.findall(rb"^#define (\S+)(?: ([^\n]*))?$", raw, re.MULTILINE))
    require(b"NDEBUG" not in macros and b"assert(expr)" in macros, "Assertions must remain active")
    for key, value in ((b"__GNUC__", b"15"), (b"__GNUC_MINOR__", b"2"), (b"__GNUC_PATCHLEVEL__", b"0"),
                       (b"__x86_64__", b"1"), (b"ONLINE_JUDGE", b"1"), (b"ATCODER", b"1")):
        require(macros.get(key) == value, "Unexpected compiler/target macro: " + key.decode())
    isa = {key.decode(): value.decode() for key, value in macros.items()
           if key.startswith((b"__AVX", b"__BMI"))}
    require(not isa if mode == "generic" else isa.get("__AVX2__") == "1" and isa.get("__BMI2__") == "1",
            "Target ISA changed; do not downgrade or retry")
    return {"sha256": sha(raw), "isa_compile_macros": isa, "assertions": True}


class Commands:
    def __init__(self, directory):
        self.directory, self.count = directory, 0
        self.deadline = time.monotonic() + JOB_SECONDS
        self.environment = {"PATH": "/usr/bin:/bin", "HOME": str(directory), "TMPDIR": str(directory),
                            "LC_ALL": "C", "TZ": "UTC"}

    def run(self, argv, source=b"", seconds=COMMAND_SECONDS):
        self.count += 1
        require(self.count <= MAX_COMMANDS, "Comparison command budget exceeded")
        timeout = min(seconds, self.deadline - time.monotonic())
        require(timeout > 0, "Comparison runtime budget exhausted")
        argv = [str(value) for value in argv]
        start = time.monotonic()
        emit("command_start", id=self.count, argv=argv, stdin_sha256=sha(source), timeout_seconds=timeout)
        with subprocess.Popen(argv, cwd=self.directory, env=self.environment, stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, start_new_session=True) as process:
            timed_out = False
            try:
                stdout, stderr = process.communicate(source, timeout=timeout)
            except subprocess.TimeoutExpired:
                timed_out = True
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                stdout, stderr = process.communicate()
        emit("command_end", id=self.count, returncode=process.returncode, timed_out=timed_out,
             seconds=time.monotonic() - start, stdout=stdout.decode(errors="replace"),
             stderr=stderr.decode(errors="replace"), stdout_sha256=sha(stdout), stderr_sha256=sha(stderr))
        require(not timed_out and process.returncode == 0, "Comparison command failed; do not retry or downgrade")
        return stdout, stderr


def git(*args):
    return subprocess.check_output(["git", "-C", str(ROOT), *args], timeout=30,
                                   env={"PATH": "/usr/bin:/bin", "LC_ALL": "C", "GIT_CONFIG_NOSYSTEM": "1"})


def checked_source(base, candidate):
    for ref in (base, candidate):
        require(git("rev-parse", "--verify", ref + "^{commit}").decode().strip() == ref,
                "Cannot resolve exact source commit")
    require(git("rev-parse", "HEAD").decode().strip() == candidate, "Checkout is not exact candidate")
    git("merge-base", "--is-ancestor", base, candidate)
    require(not git("diff", "--name-only", "HEAD", "--")
            and not git("ls-files", "--others", "--exclude-standard"), "Checkout must be clean")
    require(git("diff", "--name-only", base, candidate, "--", *LIBRARIES).decode().splitlines() == [HEADER],
            "Only the reviewed FPS production header may differ")


def dependency_proof(depfile, source, shared, snapshot, contents):
    raw = depfile.read_bytes()
    text = raw.decode().replace("\\\n", " ")
    require(text.startswith("fps-output: "), "Unexpected dependency rule")
    tokens = text.removeprefix("fps-output: ").split()
    require(len(tokens) == 6 and all(re.fullmatch(r"/[A-Za-z0-9_./+-]+", item) for item in tokens),
            "Unexpected dependency paths")
    paths = {Path(item).resolve(strict=True) for item in tokens}
    expected = {source.resolve(), (shared / "api.hpp").resolve()} | {
        (snapshot / name).resolve() for name in DEPENDENCIES}
    require(paths == expected, "Compiler did not consume the exact frozen source/dependency snapshot")
    require(all((snapshot / name).read_bytes() == data for name, data in contents.items()),
            "Snapshot source changed during compilation")
    return {"depfile_sha256": sha(raw), "files": {str(path): sha(path.read_bytes()) for path in sorted(paths)}}


def compare(base, candidate):
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_bytes(), object_pairs_hook=unique)
    validate_context(base, candidate, os.environ, event)
    checked_source(base, candidate)
    controls = [SOURCE + name for name in (*FROZEN, "run.py")] + [".github/workflows/fps-scalar-return-final.yml"]
    control_bytes = {name: git("show", candidate + ":" + name) for name in controls}
    require(all((ROOT / name).read_bytes() == data for name, data in control_bytes.items()),
            "Runner/workflow/benchmark source differs from exact candidate")
    require(all(sha(control_bytes[SOURCE + name]) == value for name, value in FROZEN.items()),
            "Frozen benchmark C++ source changed")
    sources = {variant: {name: git("show", ref + ":" + name) for name in DEPENDENCIES}
               for variant, ref in (("A", base), ("B", candidate))}
    validate_sources(sources["A"], sources["B"])
    with open("/tmp/cp-fps-tuning.lock", "a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        with Path("/tmp/cp-fps-scalar-return-" + candidate + ".started").open("x") as marker:
            marker.write(candidate + "\n")
        with tempfile.TemporaryDirectory(prefix="cp-fps-scalar-return-") as temporary:
            execute(base, candidate, Path(temporary), sources, control_bytes)


def execute(base, candidate, directory, sources, control_bytes):
    commands = Commands(directory)
    compiler = Path("/usr/bin/g++-15").resolve(strict=True)
    version, _ = commands.run([compiler, "-dumpfullversion"], seconds=30)
    require(version.strip() == b"15.2.0", "Exact GCC 15.2.0 required; do not install, substitute, or retry")
    cc1_raw, _ = commands.run([compiler, "-print-prog-name=cc1plus"], seconds=30)
    cc1 = Path(cc1_raw.decode().strip())
    require(cc1.is_absolute(), "Compiler did not resolve an absolute cc1plus")
    compiler_hashes = {str(path): sha(path.read_bytes()) for path in (compiler, cc1.resolve(strict=True))}
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    shared = directory / "driver"
    shared.mkdir()
    frozen_files = {}
    for name in FROZEN:
        target = shared / name
        target.write_bytes(control_bytes[SOURCE + name])
        frozen_files[target] = sha(target.read_bytes())
    snapshots = {}
    for variant, contents in sources.items():
        snapshots[variant] = directory / variant
        for name, data in contents.items():
            target = snapshots[variant] / name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            frozen_files[target] = sha(data)
    emit("fps_scalar_return_comparison", base=base, candidate=candidate, ref=REF,
         variants={"A": "baseline", "B": "candidate"},
         trees={variant: git("rev-parse", ref + "^{tree}").decode().strip()
                for variant, ref in (("A", base), ("B", candidate))},
         compiler=str(compiler), compiler_version="15.2.0", compiler_sha256=compiler_hashes,
         source_sha256={variant: {name: sha(data) for name, data in contents.items()}
                        for variant, contents in sources.items()},
         controls_sha256={name: sha(data) for name, data in control_bytes.items()},
         cpu=cpu, affinity=sorted(os.sched_getaffinity(0)), uname=list(os.uname()),
         cpu_model=next(line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines()
                        if line.startswith("model name")),
         cases=CASES, seed=731, scalar=17, pairs=PAIRS, warmup_processes_per_variant_case_mode=1,
         coefficient_type="public fps_mint; internal is an alias, not separate coverage",
         api_scope="result default construction, API call, allocations, move assignment, full coefficient checksum, aggregate consumption, destruction",
         excluded="input generation and independent integer-modular oracle; separate full warm-up process",
         generic_runtime_dispatch="unchanged; target-attributed/runtime NTT SIMD may still execute",
         claims="checksum-inclusive API screen; division was inconclusive locally; not kernel-only, judge-fastest, or Mac performance evidence",
         max_commands=MAX_COMMANDS, max_seconds=JOB_SECONDS, max_log_bytes=MAX_LOG_BYTES,
         command_seconds=COMMAND_SECONDS, environment=commands.environment)
    for mode, extra in MODES.items():
        flags = COMMON + extra
        macros, _ = commands.run([compiler, *flags, "-dM", "-E", "-x", "c++", "-"],
                                 b"#include <cassert>\n", seconds=30)
        emit("macro_proof", mode=mode, flags=flags, **macro_proof(macros, mode))
        binaries = {}
        for variant in ("A", "B"):
            objects = []
            for name in ("api.cpp", "bench.cpp"):
                source = shared / name
                stem = directory / (mode + "-" + variant + "-" + name)
                obj, depfile = stem.with_suffix(".o"), stem.with_suffix(".d")
                commands.run([compiler, *flags, "-I", snapshots[variant], "-MMD", "-MF", depfile,
                              "-MT", "fps-output", "-c", source, "-o", obj])
                proof = dependency_proof(depfile, source, shared, snapshots[variant], sources[variant])
                frozen_files[obj] = sha(obj.read_bytes())
                frozen_files[depfile] = sha(depfile.read_bytes())
                emit("object", mode=mode, variant=variant, source=name, object_sha256=frozen_files[obj], dependencies=proof)
                objects.append(obj)
            binary = directory / (mode + "-" + variant)
            commands.run([compiler, *flags, *objects, "-o", binary])
            binaries[variant] = binary
            frozen_files[binary] = sha(binary.read_bytes())
            emit("binary", mode=mode, variant=variant, binary_sha256=frozen_files[binary],
                 separate_translation_units=True, noinline_noipa=True, lto=False)
        for operation, (size, repeats) in CASES.items():
            rows = []
            schedule = [("warmup", None, "AB")] + [("timed", index, order) for index, order in enumerate(PAIRS)]
            for phase, pair, order in schedule:
                for variant in order:
                    binary = binaries[variant]
                    require(sha(binary.read_bytes()) == frozen_files[binary], "Benchmark binary changed")
                    raw, stderr = commands.run([binary, "public", operation, size, repeats])
                    require(not stderr, "Unexpected benchmark stderr")
                    row = validate_row(raw, operation)
                    emit("sample", mode=mode, phase=phase, pair=pair, order=order, variant=variant, **row)
                    if phase == "timed":
                        rows.append({"pair": pair, "variant": variant, **row})
            ratios = [next(row["ns_per_call"] for row in rows if row["pair"] == pair and row["variant"] == "A") /
                      next(row["ns_per_call"] for row in rows if row["pair"] == pair and row["variant"] == "B")
                      for pair in range(4)]
            emit("case", mode=mode, operation=operation, paired_baseline_over_candidate=ratios,
                 median_paired_ratio=statistics.median(ratios), adverse_pairs=[index for index, ratio in enumerate(ratios) if ratio <= 1],
                 interpretation="All pairs retained; no minimum-speedup claim or selective rerun")
        require(all(sha(path.read_bytes()) == value for path, value in frozen_files.items()),
                "Frozen source/dependency/object/binary changed")
    require(commands.count == MAX_COMMANDS, "Frozen process schedule changed")
    require(all(sha(Path(path).read_bytes()) == value for path, value in compiler_hashes.items()), "Compiler changed")
    require(all((ROOT / name).read_bytes() == data for name, data in control_bytes.items()), "Source controls changed")
    checked_source(base, candidate)
    emit("fps_scalar_return_comparison_completed", base=base, candidate=candidate, commands=commands.count,
         warmup_processes=16, timed_processes=64, all_frozen_checksums_match=True,
         source_and_binary_hashes_unchanged=True, cache_written=False, artifacts_uploaded=False)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--base-ref", required=True)
    parser.add_argument("--candidate-ref", required=True)
    args = parser.parse_args()
    compare(args.base_ref, args.candidate_ref)
