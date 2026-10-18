pow native Mint input probe (not adopted)

The candidate copies only min(input, n, log_block(n)) Mint words before
the initial inverse, then reads the public const Mint input directly in
the log derivative. c/e scaling, sinks, transformed observers, and
exp_newton are unchanged. The production implementation was not changed.

report.txt contains the decision and median timings. results.json retains
all 21 measured trials, seeded execution order, compiler flags, CPU data,
input/checker hashes, and source/binary hashes. metadata.json is the build
metadata before timing. checks.json records the guard-page/sanitizer tests
and all 37 official checker cases. build.log, check.log, and bench.log are
the original logs. manifest.json is the original measurement manifest;
its reference_kernel.cpp entry refers to the file generated below.
package-manifest.json hashes the files retained in this directory.

The only baseline-to-candidate source changes are these two patches,
applied in this order to a git archive of 4765ceb:

  patch -p1 < fps-blocked-native.patch
  patch -p1 < pow-native.patch

The first patch contains the MintSource/log input change used by this
candidate. The second adds power_native and changes the native_power
wrapper. No full source snapshots or compiled binaries are retained.

Reconstruct in a fresh directory without modifying a checkout:

  python3 test/benchmark/results/fps_native_input/pow/prepare.py \
    /path/to/cp-library /tmp/fps-pow-native-reproduce

prepare.py runs git archive 4765ceb, copies it for the candidate, applies
both patches, and checks every baseline/candidate header against the
recorded hashes (170 headers for each). It also reconstructs the reference
kernel from the retained unmodified submission source reference409718.cpp
and ref_kernel_tail.cpp, and checks both reference source hashes.
reconstruction.json records this verification. The original build/check/
bench scripts are retained byte for byte; prepare.py creates *_local.py
copies with only workspace, reference, and judge paths relocated.

Library Checker submission: https://judge.yosupo.jp/submission/409718
The reference kernel calls poly::power with a distinct const canonical
uint32 input. The library kernel calls public const fps::pow, including
Mint output conversion. Both use rng(20261019), f[0]=1, exponent 10^18,
and the recorded flags; the timer includes output/arena/transform setup
and excludes input generation and result hashing. The whole-program
comparison uses the unmodified submission source and actual official
checker on the same regular-file inputs and outputs.

Provide the generated official problem tree at:

  /tmp/library-checker-problems/polynomial/pow_of_formal_power_series

or pass --judge-dir /path/to/pow_of_formal_power_series to prepare.py.
The checker plus in/ and out/ are required by check_local.py and
bench_local.py. From a Library Checker problems checkout, the usual
generation command is ./generate.py -p pow_of_formal_power_series.
prepare.py checks all 37 original input hashes when they are available.
The raw records also retain answer and checker binary hashes for the
timed max_random_00 and max_random_01 cases.

Build and validate every variant before measuring:

  python3 /tmp/fps-pow-native-reproduce/build_local.py
  python3 /tmp/fps-pow-native-reproduce/check_local.py
  python3 /tmp/fps-pow-native-reproduce/bench_local.py

The scripts take shared /tmp/cp-fps-tuning.lock for all compiles/tests and
exclusive access for timing. They do not upgrade a held shared lock.
Timing runs on CPU 2; choose a machine where that CPU is available.
The reproduction scripts write new results only in the fresh workspace.
They do not overwrite the retained original records in this directory.

The recorded GCC is Debian 14.2.0-19. Assertions were enabled. Each case
used one warmup and 21 shuffled measured trials. Kernel sizes were
262144, 262145, 393217, 500000, followed by official max_random_00 and
max_random_01 whole-program runs. Every timed whole-program result was
validated using the actual official checker and official answer.

Guard-page tests covered short/full/longer-than-output const inputs with
exact readable extent and no padding, including non-32-byte alignment,
lazy Mint words near 2P, exp/log block boundaries, and n up to 500000.
Candidate results matched the unchanged canonical-word power function.
Existing fps_exp_blocked and fps_algorithms units passed; guard-page and
fps_exp_blocked units passed ASan/UBSan. An independent source review found
no prefix lifetime, callback ordering, padding, or observer alias issue.
