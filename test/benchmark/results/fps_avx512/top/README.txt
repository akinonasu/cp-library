No production change. All candidates were rejected after measurement.

Baseline: bd951dce6fdcb3ea9db4fcef342147012a8ab0c0
Compiler/CPU/official commit: metadata.json
Complete measurements and checks: raw/
Candidate changes: patches/ (all relative to the baseline)
Actual measurement/probe sources: harnesses/
Unmodified references: references/*.cpp.gz (source hashes in metadata.json)
Dependency snapshots: header_hashes.json

Original runs used /tmp/fps-avx512-top-base and /tmp/fps-avx512-top.
The old harnesses retain these paths to document exact build commands.
fps_ntt_round3.py accepts arbitrary reconstructed paths instead.

Reconstruction, with no compilation or timing:

  python3 test/benchmark/results/fps_avx512/top/reconstruct.py \
    --work /workspace/fps-simd-reproduction

The work directory must not already exist. This exports the baseline from the
local Git history, applies all 8 patches to separate trees, checks 175 header
hashes per tree, and expands/validates the four original reference sources.
The saved raw/reconstruction.json and reconstruction.log record the successful
independent reconstruction performed when this package was made.

Example kernel comparison:

  python3 test/benchmark/fps_ntt_round3.py \
    --baseline-root /workspace/fps-simd-reproduction/base \
    --variant wide=/workspace/fps-simd-reproduction/wide_kernel \
    --variant late2=/workspace/fps-simd-reproduction/reordered_late2 \
    --kernel inv sqrt log exp pow --sizes 262144 500000 --trials 21 \
    --work /workspace/fps-simd-reproduction/kernel \
    --output /workspace/fps-simd-reproduction/kernel.json

Example whole-process reference comparison:

  python3 test/benchmark/fps_ntt_round3.py \
    --baseline-root /workspace/fps-simd-reproduction/base \
    --variant late2=/workspace/fps-simd-reproduction/reordered_late2 \
    --references /workspace/fps-simd-reproduction/references \
    --whole convolution_mod inv_of_formal_power_series --trials 9 \
    --work /workspace/fps-simd-reproduction/whole \
    --output /workspace/fps-simd-reproduction/whole.json

Use --whole sqrt_of_formal_power_series --cases max_random_01 in a separate
invocation for square root. Original 409728 aborts on max_random_00 in this
environment, where the correct output is -1. The failed warmup is retained in
raw/whole_late2.log and raw/sqrt_reference_abort.json, not counted as a timing.
Use --whole log_of_formal_power_series and the wide (guarded) tree for the
21-trial wide512 whole-process comparison.

The official input cache defaults to /tmp/library-checker-problems. Recreate
it at the official_commit recorded in metadata.json and generate the named
problems under shared /tmp/cp-fps-tuning.lock. The new harness itself acquires
shared lock for compilation, releases it, then takes exclusive lock and pins
CPU2 for timing. It checks every whole-process output in input/actual/answer
order. A page-cached ordinary file is used for stdin/stdout, not /dev/null.

Arithmetic probes can be rebuilt under shared lock with:

  flock -s /tmp/cp-fps-tuning.lock g++ -std=c++23 -O2 -Wall -Wextra -Werror -march=haswell \
    -I/workspace/fps-simd-reproduction/wide test/benchmark/results/fps_avx512/top/harnesses/verify.cpp \
    -o /workspace/fps-simd-reproduction/verify4
  flock -s /tmp/cp-fps-tuning.lock g++ -std=c++23 -O2 -Wall -Wextra -Werror -march=haswell \
    -I/workspace/fps-simd-reproduction/wide test/benchmark/results/fps_avx512/top/harnesses/verify2.cpp \
    -o /workspace/fps-simd-reproduction/verify2
  flock -s /tmp/cp-fps-tuning.lock g++ -std=c++23 -O2 -Wall -Wextra -Werror -march=haswell \
    -I/workspace/fps-simd-reproduction/window16 test/benchmark/results/fps_avx512/top/harnesses/verify_window16.cpp \
    -o /workspace/fps-simd-reproduction/verify_window16

Run all three under the same shared lock. verify.cpp also checks late2 by
switching its include root. raw/final_checks.json records successful runs,
binary hashes, and unchanged header snapshots. Window16 checks 200000
product/square cases with exact PROT_NONE boundaries; inverse probes cover
all Half modes, zero/nonzero scales, and canaries. verify_initial_build_error
is a corrected probe namespace-alias error, not a candidate failure.

raw/source_hash_audit.json matches every measured changed-header hash to the
archived tree. wide_kernel.patch is the first version before an explicit odd-h
fallback check was added; wide.patch is the guarded version used for whole
timing and final probes. Actual Transform callers use power-of-two sizes.

The initial kernel JSON format retains samples and build hashes but only
asserts checksum equality in its harness; the newer fps_ntt_round3.py also
records the common checksum. The process comparison keeps output hashes and
checks every output. Trial ordering is reproducible from the recorded seeds.

All quoted speeds are local measurements. AVX-512 gains on this Zen4 CPU
would not establish improvements on a Yosupo machine without AVX-512.
