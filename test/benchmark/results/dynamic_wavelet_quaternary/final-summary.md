# ABC343 F final benchmark revision 2

This final frozen revision includes uniform-row query skipping. All timings are local CPU2 measurements, not judge results.

63 full-output preflights (3 small and 6 large cases across 7 executables), 42 warmups, and 294 measured processes passed. Each large case has N=Q=200000 with 100000 queries. Seven shuffled measured rounds followed one warmup per executable. Assertions remained enabled. Timing used a blocking process wait with regular input/output files, and CPU2 execution was isolated through the shared exclusive benchmark lock. Median wall time includes parsing, build, operations, output, and process startup. API operation CPU excludes loading, answer allocation, build, and answer output.

| Case | Baseline wall ms | Final wall ms | Faithful Nim wall ms | Final fast I/O + count wall ms | Baseline operations CPU ms | Final operations CPU ms | Nim operations CPU ms |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| random30 | 820.642 | 481.171 | 659.897 | 456.906 | 771.666 | 433.350 | 626.589 |
| duplicates16 | 227.732 | 109.509 | 322.403 | 89.072 | 194.824 | 73.456 | 292.039 |
| uniform_same | 48.410 | 37.704 | 217.830 | 20.801 | 20.256 | 5.627 | 191.581 |
| no_op_random | 530.403 | 323.762 | 308.287 | 295.794 | 483.588 | 277.063 | 282.321 |
| clustered | 801.782 | 472.251 | 642.946 | 453.078 | 760.925 | 425.729 | 609.680 |
| short | 788.721 | 453.254 | 592.189 | 426.792 | 735.663 | 405.616 | 560.603 |

The unchanged original C++ caller is faster than the old library in all six cases (22.1–51.9% lower wall time). It is faster than the faithful Nim solver in five cases. On no_op_random, its whole-solver median is 5.0% higher than Nim even though the instrumented operation CPU median is 1.9% lower. The separate fast I/O + count consumer control beats Nim in all six cases; those consumer changes are not a pure library speedup. API operation CPU medians are lower than Nim in all six cases. These are descriptive local comparisons and do not establish an AtCoder ranking.

The former uniform-input regression is repaired: revision 1 measured 81.211 ms, revision 2 measured 37.704 ms, and the revision 2 old-library control measured 48.410 ms. Revision 1 is retained unchanged in the parent directory.

C++ uses the original signed int/32-bit matrix caller (`kth_largest(0)`, `prev_value`, `range_freq`). The faithful Nim source uses unsigned/nonnegative 30-bit values and `count`. C++ build flags are `-std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -UNDEBUG -fno-lto -Wall -Wextra -Werror`. Component-scoped GCC O3 is part of the final headers. The Nim binary is reconstructed from the pinned original library and final main, with checks-off scoped immediately before main, using the official Nim 2.2.4 / GCC C++ backend flags. Earlier diagnostic Nim binaries were not used.

Input/output hashes, frozen source/header hashes, compiler and executable hashes, complete command/order records, all wall/child-CPU samples, build timings, and instrumented operation timings are in `results.json`. Frozen source copies are in `sources`, reference source/binary/provenance copies are in `reference`. `measurement_run.py` is an exact copy of the actual measurement runner. The separate helper-compatible adapter remains `../run_reproduce.py` and did not produce these measurements.

Measured runner SHA256: `fc55f510ab50b9d35b122714a46408b10a030b7f15f9aa36b2afc0ac82b82b57`.

Final quad vector SHA256: `541466cdfeabc524243dfcd661a752c2f3eeb71f1ffdd5b975d0265114a44d06`.

Final quad matrix SHA256: `de93626cab81a07ac4f356a7699a39beacde634889e9ddedc6d57fce387ee655`.
