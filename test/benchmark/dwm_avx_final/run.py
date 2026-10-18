#!/usr/bin/env python3
"""Fixed, stdout-only final comparison. No pilots, retries, or retuning."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import resource
import shlex
import shutil
import subprocess
import sys
import tempfile
import time

BASE = "986b5b17c622a2c999ec20e2bf5dfb1c8ecd0cb9"
HEADERS = ("data_structure/dynamic_wavelet_matrix.hpp", "components/dynamic_bit_vector.hpp")
COMMON = ["-O2", "-std=gnu++23", "-fmodules", "-ftrivial-auto-var-init=zero",
          "-DONLINE_JUDGE", "-DATCODER", "-UNDEBUG", "-fno-lto"]
TRACKS = {"native": ["-march=native"], "scalar": ["-march=x86-64", "-mtune=generic",
          "-mno-avx", "-mno-avx2", "-mno-avx512f", "-mno-bmi", "-mno-bmi2",
          "-fno-tree-loop-vectorize", "-fno-tree-slp-vectorize"]}
CASES = [(kind, n, "insert") for n in (10000, 100000)
         for kind in ("set", "simplemixed", "access")]
CASES += [(kind, 100000, "bulk") for kind in ("set", "simplemixed")]
REPS = {"set": 4, "simplemixed": 8, "access": 16}
ORDER = "AA" + "ABBA" + "BB" + "BAAB"
Q, SEED, FLOOR = 200000, 20261017, 1.0
SOURCE = Path(__file__).resolve().parent
START = time.monotonic()
COMMANDS = 0

def emit(event, **values):
    print(json.dumps(dict(event=event, **values), sort_keys=True), flush=True)

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def child_cpu():
    usage = resource.getrusage(resource.RUSAGE_CHILDREN)
    return usage.ru_utime + usage.ru_stime

def run(argv, env=None):
    global COMMANDS
    COMMANDS += 1
    argv = list(map(str, argv))
    start, cpu = time.monotonic(), child_cpu()
    try:
        result = subprocess.run(argv, capture_output=True, text=True, timeout=120, env=env)
    except subprocess.TimeoutExpired:
        emit("command_timeout", argv=argv, wall=time.monotonic()-start,
             cpu=child_cpu()-cpu, limit=120)
        raise
    emit("command", argv=argv, returncode=result.returncode,
         cpu=child_cpu()-cpu, wall=time.monotonic()-start,
         stdout=result.stdout.strip(), stderr=result.stderr.strip())
    result.check_returncode()
    return result.stdout.strip()

def main():
    if not __debug__:
        raise RuntimeError("Run without Python optimization so all exact checks remain enabled")
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--candidate-sha", required=True)
    parser.add_argument("--cxx", default="g++-15")
    args = parser.parse_args()
    repo = args.repo.resolve()
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(repo/".verify-helper"))
    import affected
    affected.verify_push_binding(repo, json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text()),
                                 os.environ, "refs/heads/codex/dwm-avx-update-20261018")
    cxx = shutil.which(args.cxx)
    if not cxx:
        raise RuntimeError("GCC 15.2.0 compiler not found")
    assert run(["git", "-C", repo, "rev-parse", "HEAD"]) == args.candidate_sha
    run(["git", "-C", repo, "diff", "--exit-code", args.candidate_sha, "--", *HEADERS])
    assert run([cxx, "-dumpfullversion", "-dumpversion"]) == "15.2.0", "GCC 15.2.0 required"
    version = run([cxx, "--version"])
    allowed = sorted(os.sched_getaffinity(0))
    cpu = allowed[0]
    os.sched_setaffinity(0, {cpu})
    info = Path("/proc/cpuinfo").read_text().split("\n\n")
    info = next(part for part in info if part.splitlines()[0].split(":")[1].strip() == str(cpu))
    assert "avx2" in info.split(), "Native AVX2 track requires an AVX2 host"
    info = {k.strip(): v.strip() for k, v in (line.split(":", 1) for line in info.splitlines())}
    emit("provenance", base=BASE, candidate=args.candidate_sha, compiler=cxx,
         compiler_sha256=sha(cxx), compiler_version=version, common_flags=COMMON,
         tracks=TRACKS, cpu=cpu, allowed_cpus=allowed, cpu_model=info["model name"], cpu_flags=info["flags"],
         sources={p.name: sha(p) for p in sorted(SOURCE.glob("*.cpp"))},
         driver_sha256=sha(__file__), cases=CASES, repetitions=REPS, q=Q, seed=SEED,
         value_type="uint32_t", bits=20, distribution="uniform", locations="random",
         cache="warm", slots=ORDER, sample_floor_seconds=FLOOR,
         timed_processes=192, exact_preflights=32, distinct_oracle_inputs=6,
         timed_operations=326400000, command_timeout_seconds=120)
    expected, clean_env = {}, dict(os.environ)
    for key in ("DWM_EXPECTED_HASH", "DWM_EXPECTED_FINAL_HASH", "DWM_VALIDATE_REPLAYS"):
        clean_env.pop(key, None)
    with tempfile.TemporaryDirectory(prefix="dwm-avx-final-") as directory:
        work = Path(directory)
        base = work / "base"
        base.mkdir()
        run(["git", "-C", repo, "archive", "--format=tar", "--output="+str(work/"base.tar"), BASE, *HEADERS])
        run(["tar", "-xf", work/"base.tar", "-C", base])
        escape, oracle = work/"escape.o", work/"vector_oracle"
        run([cxx, *COMMON, "-march=x86-64", "-c", SOURCE/"escape.cpp", "-o", escape])
        run([cxx, *COMMON, "-march=x86-64", "-MMD", "-MF", work/"oracle.d",
             SOURCE/"vector_oracle.cpp", "-o", oracle])
        assert ".hpp" not in (work/"oracle.d").read_text(), "Oracle must not include production headers"
        emit("oracle_binary", sha256=sha(oracle), bytes=oracle.stat().st_size,
             source_sha256=sha(SOURCE/"vector_oracle.cpp"))
        for track, isa in TRACKS.items():
            binaries = {}
            for arm, include in (("A", base), ("B", repo)):
                for kind in REPS:
                    name = track+"-"+arm+"-"+kind
                    binary, obj, dep = work/name, work/(name+".o"), work/(name+".d")
                    source = SOURCE/("bench.cpp" if kind == "set" else "caller.cpp")
                    shape = [] if kind == "set" else ["-DCALLER_SHAPE="+str(1 if kind == "access" else 2)]
                    flags = COMMON+isa+["-DBITS=20", "-DVALUE_TYPE=std::uint32_t"]+shape
                    run([cxx, *flags, "-I"+str(include), "-MMD", "-MF", dep, "-c", source, "-o", obj])
                    dependencies = {Path(p).resolve() for p in shlex.split(dep.read_text().replace("\\\n", " ").split(":", 1)[1])}
                    selected = {p for p in dependencies if p.name in ("dynamic_wavelet_matrix.hpp", "dynamic_bit_vector.hpp")}
                    assert selected == {(include/p).resolve() for p in HEADERS}, "Wrong production headers selected"
                    run([cxx, *flags, obj, escape, "-o", binary])
                    emit("binary", track=track, arm=arm, kind=kind, sha256=sha(binary),
                         bytes=binary.stat().st_size, flags=flags,
                         selected_headers={str(p): sha(p) for p in sorted(selected)})
                    binaries[arm, kind] = binary
            for kind, n, construction in CASES:
                cli_kind = "updates" if kind == "set" else kind
                prefix = [cli_kind, str(n), str(Q), str(SEED), "uniform", "random"]
                case = dict(track=track, kind=kind, n=n, construction=construction)
                reference_input = None
                for arm in "AB":
                    dump = work/(track+"-"+arm+"-"+kind+"-"+str(n)+"-"+construction)
                    result = json.loads(run([binaries[arm, kind], *prefix, "1", "warm", dump, construction], clean_env))
                    inputs, outputs = Path(str(dump)+".input").read_bytes(), dump.read_bytes()
                    if reference_input is None:
                        reference_input = inputs
                    assert inputs == reference_input, "Arms generated different exact inputs"
                    input_hash = hashlib.sha256(inputs).hexdigest()
                    key = (input_hash, "uint32_t/Bits20", cli_kind)
                    if key not in expected:
                        run([oracle, dump, cli_kind])
                        expected[key] = (inputs, outputs, result["hash"], result["finalhash"])
                    truth = expected[key]
                    assert inputs == truth[0] and outputs == truth[1], "Exact preflight mismatch"
                    assert (result["hash"], result["finalhash"]) == truth[2:], "Preflight digest mismatch"
                    emit("preflight", **case, arm=arm, input_sha256=input_hash,
                         full_output_sha256=hashlib.sha256(outputs).hexdigest(), status="pass")
                    dump.unlink()
                    Path(str(dump)+".input").unlink()
                samples = []
                env = dict(clean_env, DWM_EXPECTED_HASH=truth[2], DWM_EXPECTED_FINAL_HASH=truth[3])
                for slot, arm in enumerate(ORDER):
                    result = json.loads(run([binaries[arm, kind], *prefix, str(REPS[kind]),
                                             "warm", "-", construction], env))
                    assert (result["hash"], result["finalhash"]) == truth[2:], "Timed digest mismatch"
                    samples.append(result)
                    emit("sample", **case, slot=slot, arm=arm, input_sha256=input_hash, **result)
                contrasts, descriptive = {}, {}
                for metric in ("cpu", "wall"):
                    values = [sample[metric] for sample in samples]
                    contrasts[metric] = [(values[3]+values[4])/(values[2]+values[5]),
                                         (values[8]+values[11])/(values[9]+values[10])]
                    descriptive[metric] = {"AA_second_over_first": values[1]/values[0],
                                           "BB_second_over_first": values[7]/values[6]}
                emit("summary", **case, B_over_A_ABBA_BAAB=contrasts, same_arm=descriptive,
                     below_floor=any(min(s["cpu"], s["wall"]) < FLOOR for s in samples),
                     adverse=any(r > 1 for ratios in contrasts.values() for r in ratios),
                     note="All rows retained; descriptive comparisons, not fine-percent equivalence evidence")
        assert len(expected) == 6
    emit("complete", commands=COMMANDS, cpu=child_cpu()+time.process_time(), wall=time.monotonic()-START)

if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        emit("failed", error=str(error), commands=COMMANDS, cpu=child_cpu()+time.process_time(), wall=time.monotonic()-START)
        raise
