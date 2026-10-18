log MintSource input fusion

Source-only reproduction. No binaries or full snapshot are committed.
Base commit: 4765ceb5f995801d27b23840392c9c354ecdecb9.
Reference: https://judge.yosupo.jp/submission/409452
The unmodified reference source is retained as reference409452.cpp.

From the repository root, reconstruct in a new directory:

  python3 test/benchmark/results/fps_native_input/log/prepare.py \
    "$PWD" /tmp/fps-log-native-reproduction

prepare.py uses git archive 4765ceb to create before, after, after_layout,
and after_shared, applying each candidate patch independently to before.
It checks the recorded header hashes, reference source hash, and official
max_random input hashes when present. reconstruction.json records the
checks. *_local.py copies relocate workspace, judge, and reference paths;
the retained original measurement scripts remain unchanged.
--judge-dir can select another generated log_of_formal_power_series tree.

Candidate patches:
  after: split-input.patch (first candidate, direct input at all sizes)
  after_layout: old-layout.patch (preserve the original large allocation)
  after_shared: shared-input.patch (adopted, shared body, n >= 262144 only)

Official problems commit: 1814c4e5205517e368bb57a8d1127eb961cfeaae.
Provide generated checker, in/, and out/ at:
  /tmp/library-checker-problems/polynomial/log_of_formal_power_series
For a fresh environment:

  git clone https://github.com/yosupo06/library-checker-problems /tmp/library-checker-problems
  git -C /tmp/library-checker-problems checkout 1814c4e5205517e368bb57a8d1127eb961cfeaae
  flock -s /tmp/cp-fps-tuning.lock \
    python3 /tmp/library-checker-problems/generate.py -p log_of_formal_power_series

Run the local scripts in this order:

  python3 /tmp/fps-log-native-reproduction/bench_local.py
  python3 /tmp/fps-log-native-reproduction/layout_local.py
  python3 /tmp/fps-log-native-reproduction/shared_bench_local.py
  python3 /tmp/fps-log-native-reproduction/repeat_local.py

bench_local.py builds before/after kernels and whole programs plus the
unmodified reference whole program. Later scripts reuse these binaries.
Compile/test takes the shared advisory lock /tmp/cp-fps-tuning.lock;
timed regions take its exclusive lock after builds release the shared lock.
CPU 2, GCC 14.2.0, C++23, -O2, -DEVAL, -DONLINE_JUDGE,
-ftrivial-auto-var-init=zero, -march=haswell, assertions enabled.
21 measured trials follow a warmup, seeded shuffled order 20261019;
the additional 99-trial whole run uses shuffle seed 20261020.

Whole trials call the actual checker with arguments:
  checker input actual_output official_answer
Boundary inputs are prefixes of official max_random_00; their expected
answers are generated using the unchanged reference submission 409452.
All outputs from every measured whole trial passed the checker.
kernel.cpp calls the public const fps::log API, including output/arena/
transform allocation and excluding input generation and output hashing.
Whole times include process start/exit and regular-file input/output.

The expanded boundary unit is retained as log-boundary-unit.cpp. It adds
262143/262144/262145/393217 identity cases and readonly exact-sized Mint
input ending at PROT_NONE, including short/non-32-byte-aligned input and
both integrate modes. Compile this snapshot against after_shared:

  flock -s /tmp/cp-fps-tuning.lock g++ -std=c++23 -O2 \
    -I/tmp/fps-log-native-reproduction/after_shared \
    /tmp/fps-log-native-reproduction/log-boundary-unit.cpp \
    -o /tmp/fps-log-native-reproduction/log-unit
  flock -s /tmp/cp-fps-tuning.lock /tmp/fps-log-native-reproduction/log-unit
  flock -s /tmp/cp-fps-tuning.lock g++ -std=c++23 -O1 -g \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    -I/tmp/fps-log-native-reproduction/after_shared \
    /tmp/fps-log-native-reproduction/log-boundary-unit.cpp \
    -o /tmp/fps-log-native-reproduction/log-unit-sanitize
  flock -s /tmp/cp-fps-tuning.lock env ASAN_OPTIONS=detect_leaks=0 \
    UBSAN_OPTIONS=halt_on_error=1 \
    /tmp/fps-log-native-reproduction/log-unit-sanitize

log-boundaries.patch is the same unit change relative to the base commit;
the archived after_shared/test/unit/fps_log.cpp still contains the older
unit until that separate patch is applied. checks.json separates the older
candidate-unit checks from the expanded production-unit checks.

results.json: first split-input candidate and unchanged submission comparison.
layout.json: allocation layout diagnostic, including rejected old-layout candidate.
shared.json: adopted shared-body candidate with n < 262144 using the old path.
repeat.json: 99-trial whole-program check using the same shared candidate binaries.
checks.json: unit/sanitizer commands and source/binary fingerprints.
reconstruction-check.json: independent restoration and full-header comparison.
*.log: original build/timing logs.

make.py and share.py retain the original prototype construction scripts.
The patches are self-contained; these scripts are not needed to restore
or reproduce the measurements. New measurement outputs stay in the
reconstruction workspace and do not overwrite the retained raw records.
