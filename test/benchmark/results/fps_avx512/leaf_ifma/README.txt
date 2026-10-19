AVX512 IFMA leaf_product experiment (rejected)

Baseline: bd951dce6fdcb3ea9db4fcef342147012a8ab0c0
No production source was changed by this experiment. The candidate is a
standalone helper in leaf_ifma.hpp. No binaries or full source snapshots
are included in this pack.

Requirements: Linux x86_64, GCC 14, AVX2 for baseline, AVX512F+AVX512IFMA
for candidate runs, CPU 2 available. The archived machine is AMD EPYC 9V74.
All compile/check commands use SH flock; benchmark uses EX flock internally.
Do not wrap bench_local.py in an SH lock. Assertions remain enabled.

Reconstruct in an empty temporary directory:
  python3 test/benchmark/results/fps_avx512/leaf_ifma/prepare.py "$PWD" /tmp/leaf-ifma-reproduce
  python3 /tmp/leaf-ifma-reproduce/build_local.py > /tmp/leaf-ifma-reproduce/build.log 2>&1
  python3 /tmp/leaf-ifma-reproduce/check_extra.py > /tmp/leaf-ifma-reproduce/extra.log 2>&1
  python3 /tmp/leaf-ifma-reproduce/bench_local.py > /tmp/leaf-ifma-reproduce/bench.log 2>&1

Run from the repository root. prepare.py archives the pinned commit,
checks every baseline header SHA256, builds independent range-instrumented
and align32 trees from the two patches, and copies the unchanged C++
sources. build.py/bench.py are original scripts with their original /tmp
path preserved byte-for-byte; prepare.py changes only that path in the
generated *_local.py scripts. check_extra.py reproduces range and align32
checks. Exact original commands/hashes are in metadata.json and checks.json.
baseline-headers.json and sourcehash.json record full header hashes.
prepare-audit.json records a fresh archive/patch reconstruction check.

Results: results.json contains all 504 measured samples and shuffle order;
raw.tsv is the same samples in tabular form. One warmup plus 21 shuffled
trials per mode, 1,048,576 leaf calls per sample, three working sets.
All modes produced the same output hash within each working set.

The runtime dispatch mode uses the supported branch on this CPU.
forced_fallback measures the old AVX2 fallback body on this CPU; it is not
an unsupported-CPU measurement. The guard checks AVX512F+IFMA before using
the target-attributed function, with Vec256/YMM return ABI preserved.

Correctness: 200,000 windows / 1,600,000 lanes checked against original
raw output and unsigned-128 scalar Montgomery oracle. Weighted P is
included. b ends exactly at a PROT_NONE page and is read-only in the final
call. Range tracing covers public inv/log/exp/sqrt/pow up to n=500000 with
lazy input words near 2P. align32 additionally checks 10,000 windows /
80,000 lanes with actual Window address mod64=32. No extra timing was
performed for the alignment variant. See range-proof.json for input bounds,
the low52 carry rule and exact overflow bounds.

Decision: reject. Best IFMA inline mode is about twice as slow as baseline
in this microbenchmark. No process-level or official fastest comparison
was run, and no process-level speed claim is made.
