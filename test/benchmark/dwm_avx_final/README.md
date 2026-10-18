# Fixed final comparison

Reproduction sources for the fixed comparison. See `RESULTS.md` for the local
candidate screen and the associated PR / Actions logs for final results.
The workflow deliberately has no result cache, artifact upload, or installation step.
It requires the runner's `g++-15` to report exactly `15.2.0`; otherwise it fails.
It triggers only on pushes to `codex/dwm-avx-update-20261018`, not on PR or main events.
The driver invokes the existing `.verify-helper/affected.py` push binding once, with
that fixed branch ref, before temporary outputs. Bytecode output is disabled before
the import. This package adds no route, whole-tree binding, or audit framework.

Base: `986b5b17c622a2c999ec20e2bf5dfb1c8ecd0cb9`, with the two needed headers exported
by `git archive`. Candidate: the exact supplied checkout SHA, with the two production
headers required to match it. Every compiled benchmark depfile must select those
exact headers. No Titan or other-library implementation is included.

The existing `bench.cpp`, `caller.cpp`, and `escape.cpp` are reused. The only harness
change adds a final `insert|bulk` argument. `insert` constructs a fresh owner by
appending through `insert(size(), value)`; `bulk` uses `build`. Construction, warming,
output hashing, full preflight output/final-state extraction, and destruction are
outside the operation timer. Opaque escape calls and the prior timing boundary stay
unchanged. Every timed repetition validates its output digest against the independently
checked preflight; the full final-state digest retains the existing first-repetition
boundary. Normal existing CI, not this driver, handles native/scalar/sanitizer/Mac tests.

Eight cases, no others:
- Set, ordinary four-operation mixed, and access at n=10,000 and 100,000, insertion-built.
- Set and ordinary mixed at n=100,000, bulk-built.

All use uint32_t/Bits20, seed 20261017, full uniform values, random locations, warm
owners, and Q=200,000. Repetitions are fixed at set=4, mixed=8, access=16. Native then
explicit scalar run sequentially in one job on the first allowed CPU, pinned once.
The scalar track disables AVX/AVX2/AVX512F, BMI/BMI2, and loop/SLP autovectorization.
Both tracks use O2, gnu++23, fmodules, zero auto-variable initialization, ONLINE_JUDGE,
ATCODER, assertions enabled, and no LTO. The unchanged independent vector oracle
includes no production headers and checks every output and final-vector word.

Fixed budget: 16 cells, 12 timed slots each (AA, ABBA, BB, BAAB), 192 timed processes,
32 exact one-repetition arm/case/flag preflights, 6 distinct oracle inputs, and 326.4
million timed operations. Oracle success is reused only for exactly matching input
bytes, hash, kind, and type; all preflight output bytes must also match. There are no
pilots, retuning, retries, discarded adverse rows, or additional driver tests.

Each row reports both whole-four-sample B/A contrasts, AA and BB second/first ratios,
CPU and wall timings, and a descriptive flag if any sample is below the fixed 1-second
floor. Neither floor rows nor regressions are removed. No fine-percent equivalence
claim is made. Source/binary/compiler/flags/input hashes, binary byte sizes, and actual command CPU/wall
times go to stdout. Temporary files exist only during this invocation.

Hard limits, not a predicted runtime: 120 seconds per subprocess, 18-minute benchmark
step, 20-minute job. Timeout/failure ends this attempt without retrying or reducing work.
