# FPS scalar return

The four scalar value-returning `+`, `-`, `*`, `/` operators use a named local instead of returning the lvalue result of compound assignment. API signatures, compound operations, coefficient operation order, scalar inversion count, polynomial wrappers, shifts, kernels, and ISA dispatch are unchanged. The second coefficient-vector copy is removed by NRVO or vector buffer transfer. No coefficient move constructor is required.

## Local screen

Baseline: `1b8607587c164b013386423ada2298e05a9e4028`. Candidate `fps/fps.hpp` SHA-256: `b29e2144d453eafa17e296ca77c603bfb8f77b04b02274bfb4d4a75d93263a34`.

Linux x86_64, AMD EPYC 9V74, GCC 14.2.0, CPU 2. Common flags: `-std=c++23 -O2 -fno-lto -ftrivial-auto-var-init=zero`; native adds `-march=native`. Assertions and existing header O3 pragmas remain enabled. Existing runtime NTT SIMD dispatch may execute in a generic build.

Four cases, seed 731, scalar 17. Repeats were frozen before any timing: +32=300000, -262144=128, *65536=512, /4096=4096. One warm-up process per variant/case/build precedes four measured pairs ordered AB, BA, AB, BA. Each measured trial is a fresh process; process-local initialization is included. No calibration, tuning, or unchanged reruns followed. `fps_mint` aliases `fps_detail::Mint`, so duplicate alias-labeled cases were removed before timing.

The measured loop includes call, allocation, result assignment, a complete-coefficient checksum, aggregate mixing, and destruction. Input/reference generation is excluded. Equal separate-TU noinline/noipa wrappers and no LTO prevent dead-code elimination. Every output coefficient is consumed, every trial's aggregate matches an independent integer-modular reference, and source checksums remain unchanged. Checksums are not collision-free proofs; correctness has separate coefficient-by-coefficient tests. No checksum cost is subtracted.

Median microseconds per call, checksum included:

| Build | Operation / size | Baseline | Candidate | Speedup | Faster pairs |
| --- | --- | ---: | ---: | ---: | ---: |
| generic | + / 32 | 0.094652 | 0.080779 | 1.1717x | 4/4 |
| generic | - / 262144 | 936.196500 | 560.724500 | 1.6696x | 4/4 |
| generic | * / 65536 | 217.291000 | 136.954000 | 1.5866x | 4/4 |
| generic | / / 4096 | 8.814790 | 8.751845 | 1.0072x | 3/4 |
| native | + / 32 | 0.095700 | 0.081461 | 1.1748x | 4/4 |
| native | - / 262144 | 919.091000 | 536.006500 | 1.7147x | 4/4 |
| native | * / 65536 | 222.237000 | 141.382500 | 1.5719x | 4/4 |
| native | / / 4096 | 9.263880 | 8.825790 | 1.0496x | 4/4 |

Generic division is inconclusive: its paired speedups are 1.0350, 1.0095, 0.9762, 1.0258, including a slower pair. Addition, subtraction and multiplication improve in every local pair. No universal speedup, pure kernel timing, judge-fastest, or end-to-end FPS algorithm claim follows from these four sizes. Raw warmups, all samples, paired ratios, source proof, and limitations are retained locally, not committed. All frozen operation cases appear above, including the slower division pair.

## Copy, allocation, and correctness

Separate normal and `-fno-elide-constructors` diagnostics show exactly one fewer allocation and n fewer coefficient copies for every nonempty operation. Retained capacity is unchanged; measured intermediate/result peak payload bytes drop from `2*n*sizeof(T)` to `n*sizeof(T)`. The peak excludes the already-created source, prewarmed caches, and allocator metadata; it is not process RSS. Measured result allocations are fully freed after destruction. Empty + and - save one allocation and one output coefficient copy; empty * and / remain allocation-free.

A nontrivial coefficient with noncanonical 64-bit encoded storage and deleted move constructor verifies generic support and vector buffer transfer without coefficient moves. Scalar parameter copies are counted separately and unchanged. Division calls its inverse once, including empty input. No new type-layout assumptions are made.

Local focused correctness covered 2,880 scalar executions and 1,998 adjacent controls across the originally requested three labels, representing only two distinct coefficient implementations. Public/internal repetitions are redundant aliases, not independent coverage. Cases include empty/singleton/boundary/random values, 0/1/-1/17 scalars where valid, source/raw representation preservation, self-assignment, and unchanged polynomial/shift controls. Normal/no-elide and focused UBSan passed. All 12 affected existing units passed normal and UBSan. The first standalone pass checked the ten FPS-directory headers; the eight transitive component/convolution headers were then checked separately, giving 18/18 without repeating prior checks.

Normal ASan/LSan was blocked by the environment's retained ptrace fatal error. A separate leak-disabled diagnostic passed; this is not a normal sanitizer pass or leak-freedom claim. The full affected scope is 18 headers/bundles, 12 units/sanitizer targets, and 22 judge drivers; the judges were not run locally.

## Target qualification

The prepared source-bound runner is `fps_scalar_return/run.py`. It preserves the same four frozen cases and complete-result checksum scope for exact GCC 15.2.0 generic/native comparison. It requires the reviewed exact source/ref/compiler, keeps assertions and no LTO, and refuses retries or compiler downgrade. It is not the broad historical FPS tuning sweep and executes no external reference submissions.

Actual target-GCC, current-candidate CI, normal sanitizer, judge and macOS checks are not established by the local screen. Mac compatibility is source-level reasoning only until actual Apple Clang checks pass at the latest candidate. Baseline CI receipts do not substitute for those checks. Historical FPS references and algorithm comparisons remain in [fps_tuning.md](fps_tuning.md); no external submitted code was executed for this wrapper-only change.

### Initial target compilation failure

The first [GCC 15.2.0 attempt](https://github.com/qandori/cp-library/actions/runs/39107815980/job/117617366393), at source `9d7ee01cc9ab6b139eb93e2d0ca9377356a8fa26`, failed at command 4 while compiling baseline generic `api.cpp`. `-Werror=free-nonheap-object` reported a nonzero-offset delete through the baseline scalar `+` wrapper and `std::vector::resize`. Compiler/source/macro checks passed, but zero warmup or timed samples ran. The original failure is retained; it provides no target speedup result.

Source review found standard vector copy, guarded empty `resize(1)`, a valid first-element update, and the temporary alive through the return copy, with no custom allocation or pointer adjustment in this path. [GCC PR115016](https://gcc.gnu.org/pipermail/gcc-bugs/2024-May/864818.html) reports an analogous vector ownership-analysis diagnostic, not a proven identification of this exact case.

The narrowly reviewed continuation adds only `-Wno-error=free-nonheap-object` to the shared benchmark flags for both variants. The warning remains visible in full command logs; every occurrence must be reviewed against the exact baseline path above before accepting the comparison. Different paths or candidate occurrences require investigation, not automatic acceptance. All other warnings remain errors. Frozen C++ sources, GCC 15.2.0, optimization/assertion/ISA flags, checksums, cases, repeats, ordering and budgets are unchanged. Candidate CI, the intrinsic gate and sanitizers retain their existing flags without this exception. A continuation would be the first actual timed target run, not a rerun selected for favorable measurements.
