Inverse/sqrt output experiments from immutable baseline 144ae323.

No full source snapshots or binaries are retained. Each patch is applied
independently to the baseline:

  live-batch.patch: candidate; SIMD Shoup result goes directly into a live
    std::array<Mint,64>, followed by out.insert. Removes the intermediate
    uint32 values[64] and the second batch copy. Scaling is unchanged.
  fused.patch: final inverse high half absorbs R^2 into the final existing
    cyclic_product scale; that half is appended as Montgomery words.
  combined.patch: live batch plus final-high-half scale fusion.
  scaled.patch: every inverse Newton approximation is kept R^2 times the
    canonical inverse; base coefficients are scaled once and each existing
    second cyclic_product uses -R^-2. All inverse output uses append<true>.

The only modified production header in these candidate trees is
components/fps_inverse_blocked.hpp. Shared blocked/io/output files are
unchanged. scaled-checks.json contains the scale proof, independent alias
review, and normal/sanitized/public unit check records for scaled.patch.

Reconstruct in a fresh workspace, from a repository containing 144ae323:

  python3 test/benchmark/results/fps_inverse_round2/prepare.py \
    /path/to/cp-library /tmp/fps-inverse-round2-reproduce

Use --judge-root /path/to/library-checker-problems/polynomial if needed.
prepare.py runs git archive, creates all five trees, applies each patch,
and compares every header with its recorded SHA-256 (174 per tree).
It reconstructs both reference kernels from the retained unmodified
submission sources and tail harnesses, and verifies their source hashes.
It also creates a separate integrated/ tree from scaled/ plus unit.patch
and verifies that the added public unit exactly matches integrated-unit.cpp
and the tested hashes in integrated-checks.json. Measured candidate trees
remain in their original unit-test state.
reconstruction.json records the verification. The original scripts are
retained byte for byte; *_local.py copies relocate workspace paths only.

Reproduce the original sequence, after generating official inputs,
answers and checkers for inv_of_formal_power_series and
sqrt_of_formal_power_series in a Library Checker problems checkout:

  ./generate.py -p inv_of_formal_power_series
  ./generate.py -p sqrt_of_formal_power_series

  python3 /tmp/fps-inverse-round2-reproduce/build_local.py
  python3 /tmp/fps-inverse-round2-reproduce/bench_local.py
  python3 /tmp/fps-inverse-round2-reproduce/fused_build_local.py
  python3 /tmp/fps-inverse-round2-reproduce/fused_bench_local.py
  python3 /tmp/fps-inverse-round2-reproduce/scaled_build_local.py
  python3 /tmp/fps-inverse-round2-reproduce/repeat_local.py
  python3 /tmp/fps-inverse-round2-reproduce/scaled_reference_local.py

All compiles/tests take shared /tmp/cp-fps-tuning.lock; timing takes its
exclusive lock after releasing shared access. Timed subprocesses inherit
CPU 2 affinity. All timed whole-program results use the actual official
checker with input, actual output, official answer arguments. The scripts
require CPU 2 to be available. Assertions remain enabled.

Measurement flags: GCC 14.2.0, -std=c++23 -O2 -DEVAL -DONLINE_JUDGE
-ftrivial-auto-var-init=zero -march=haswell. Compiler/CPU/source/binary
details are retained in metadata.json and the measurement JSON files.
Normal and sanitizer test commands are in metadata and scaled-checks.json.
The standalone append_check.cpp and fused_check.cpp cover output prefixes,
SIMD/tail lengths, short/full/long input, read-only guard-page inputs with
no padding, lazy Mint inputs, and canonical raw-word equality.

The scaled validation commands can be relocated from scaled-checks.json.
For example compile fused_check.cpp with -I<workspace>/scaled, then run
normally and with -O1 -g -fsanitize=address,undefined
-fno-omit-frame-pointer and ASAN_OPTIONS=detect_leaks=0,
UBSAN_OPTIONS=halt_on_error=1; keep shared advisory locking around both
compile and execution. These commands do not rerun during scaled_build.py.

Files:
  results.json: first live-batch 21-trial measurements, including the
    rejected sqrt _00 timing diagnostic (its official answer is -1).
  fused_results.json: 21-trial inverse fusion comparisons and sqrt
    positive-answer max_random_01/_02 and near_262144_01/_02 comparisons.
  repeat.json: 21-trial scaled inverse kernels and 99-trial positive
    whole-program stability comparisons.
  scaled_reference.json: final 21-trial baseline/scaled/reference whole
    comparison in the same shuffled trial sequence.
  stability.json: paired median differences and exact binomial-order
    95% median intervals for the 99-trial measurements.
  pooled-stability.json: supplementary 120 paired differences from the
    separate 99/21 records after matching binary hashes; the extra 21
    max_random_00 unpaired median reverses and is retained as measured.
  metadata.json, fused_metadata.json, scaled_metadata.json: build hashes.
  scaled-checks.json: extra scaled validation and representation proof.
  integrated-checks.json, integrated-unit.cpp, unit.patch: expanded public
    unit cases after adoption; normal and ASan/UBSan commands and hashes.
  *.log: original output logs. manifest.json: retained-file SHA-256 hashes.

References:
  inverse https://judge.yosupo.jp/submission/409370
  sqrt https://judge.yosupo.jp/submission/409728

Kernel input is seeded rng(20261019) with constant coefficient 1. The FPS
kernels call public const inv/sqrt and include Mint import/output work.
The inverse reference follows the submission's last in-place step with
a timed canonical input copy; sqrt reference calls poly::sqrt with a
distinct canonical input. Arena/transform/output setup is timed; input
generation and result hashing are excluded. Append microbenchmarks retain
output capacity and average 32 appends per measured subprocess.
