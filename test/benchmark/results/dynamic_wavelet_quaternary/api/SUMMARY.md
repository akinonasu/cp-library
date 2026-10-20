# Broad API controls — frozen final revision

Baseline: `2559803022319333756173295947577d6f5f9b2e`. Exact current header SHA256 and all compiler/source/binary hashes and commands are retained in `manifest.json`; measured runner is `measurement_run.py`.

All 104 small N=1100/Q=1400 independent vector-oracle checks passed. All 312 timed process outputs and final states matched between the frozen implementations (52 cases, 3 shuffled paired samples, N=100000/Q=20000). The large runs are full-output differential checks rather than independent large-oracle checks.

Operation CPU median ratios in this run range 0.243–0.902 final/base. 46/52 rows include a sample shorter than 50ms. These short 3-repeat results are descriptive controls and do not establish a universal API speedup. Cases over the descriptive 15% regression threshold: 0.

The machine is AMD EPYC 9V74; GCC14.2 flags: `-std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -UNDEBUG -fno-lto -Wall -Wextra -Werror`. Timed processes use CPU2 under EX `/tmp/cp-fps-tuning.lock`; compilation and oracle checks use SH. Construction, owner warming, input generation, output hashing/extraction and destruction are outside the operation timer.

Current/base operation CPU median ratio (lower is faster):

| API | signed64 | unsigned64 | signed31 | unsigned7 |
|---|---:|---:|---:|---:|
| access | 0.881 | 0.902 | 0.843 | 0.764 |
| updates | 0.623 | 0.625 | 0.617 | 0.660 |
| set-noop | 0.788 | 0.800 | 0.808 | 0.844 |
| insert | 0.714 | 0.722 | 0.745 | 0.831 |
| erase | 0.748 | 0.734 | 0.794 | 0.833 |
| count | 0.684 | 0.700 | 0.647 | 0.417 |
| rank | 0.795 | 0.804 | 0.746 | 0.732 |
| select | 0.731 | 0.724 | 0.631 | 0.415 |
| kth | 0.798 | 0.802 | 0.746 | 0.551 |
| freq | 0.608 | 0.672 | 0.689 | 0.500 |
| range | 0.598 | 0.646 | 0.621 | 0.492 |
| prev | 0.637 | 0.618 | 0.480 | 0.253 |
| next | 0.563 | 0.631 | 0.494 | 0.243 |

Construction and allocated-storage tradeoffs for N=100000. Build CPU time is the median across the 13 case medians for each domain; bytes are API `memory_usage()`, distinct from process RSS.

| Domain | Baseline build ms | Final build ms | Build change | Baseline bytes | Final bytes | Memory change |
|---|---:|---:|---:|---:|---:|---:|
| i64 | 10.6924 | 8.0177 | -25.0% | 1629696 | 1913088 | +17.4% |
| u64 | 10.5220 | 8.2171 | -21.9% | 1629696 | 1913088 | +17.4% |
| i31 | 4.8228 | 3.9852 | -17.4% | 789384 | 956544 | +21.2% |
| u7 | 1.1821 | 1.1822 | +0.0% | 178248 | 239136 | +34.2% |

All rows are retained in `results.json`, including full ranges and process commands. Count/rank select an existing value on half the queries. No-op sets assign their actual unchanged original values. Signed64 and unsigned64 exercise full-width public types; signed31 and unsigned7 also cover odd value widths.

The original revision1 harness had `-Wmisleading-indentation` failures in inherited adjacent driver statements; these were formatting-only and corrected before any successful measurements. No compiler flag or meaningful check was removed. Revision2 uses the same corrected driver, unchanged.
