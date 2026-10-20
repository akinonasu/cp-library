# Broad API controls — frozen revision 1

Baseline: `2559803022319333756173295947577d6f5f9b2e`. This completed run is preserved; production may advance after its frozen header hashes. `measurement_run.py` is byte-identical to the original measured `run.py` (SHA256 `54ccae03f89987ac4c592232ba1cb0a06281b1b3830cfe00b2d54eca2b5366a9`).

All 104 small N=1100/Q=1400 independent vector-oracle checks passed. All 312 timed process outputs and final states matched completely between the frozen implementations (52 domain/API cases, 3 shuffled paired samples). Large N=100000/Q=20000 cases use full-output differential validation rather than an independent large oracle.

Operation CPU medians are 0.250–0.894 of baseline in these samples; none exceeds the descriptive 1.15 regression threshold. 46/52 rows have at least one sample shorter than 50ms, so the short 3-repeat measurements do not establish a universal API speedup. Construction is timed separately and exposes the small-width tradeoff below.

CPU2, EX `/tmp/cp-fps-tuning.lock`, AMD EPYC 9V74, GCC14.2, assertions enabled. Compile/oracle work uses SH. Flags: `-std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -UNDEBUG -fno-lto -Wall -Wextra -Werror`. Input creation, construction, warming, output hashing/extraction and destruction lie outside operation timing.

Current/base operation CPU median ratio (lower is faster):

| API | signed64 | unsigned64 | signed31 | unsigned7 |
|---|---:|---:|---:|---:|
| access | 0.869 | 0.894 | 0.849 | 0.743 |
| updates | 0.633 | 0.636 | 0.627 | 0.695 |
| set-noop | 0.777 | 0.778 | 0.838 | 0.866 |
| insert | 0.717 | 0.743 | 0.747 | 0.830 |
| erase | 0.749 | 0.756 | 0.783 | 0.820 |
| count | 0.685 | 0.713 | 0.633 | 0.432 |
| rank | 0.791 | 0.781 | 0.749 | 0.730 |
| select | 0.726 | 0.732 | 0.625 | 0.421 |
| kth | 0.758 | 0.785 | 0.721 | 0.579 |
| freq | 0.641 | 0.626 | 0.683 | 0.503 |
| range | 0.599 | 0.652 | 0.690 | 0.481 |
| prev | 0.636 | 0.611 | 0.473 | 0.256 |
| next | 0.617 | 0.613 | 0.472 | 0.250 |

Build CPU and reported allocated memory, construction of N=100000 (build times: median across the 13 API-case medians):

| Domain | Baseline build ms | Final build ms | Baseline bytes | Final bytes | Increase |
|---|---:|---:|---:|---:|---:|
| i64 | 10.4593 | 9.7060 | 1629696 | 1913088 | 17.4% |
| u64 | 10.3995 | 9.9139 | 1629696 | 1913088 | 17.4% |
| i31 | 4.7009 | 4.8371 | 789384 | 956544 | 21.2% |
| u7 | 1.1515 | 1.3940 | 178248 | 239136 | 34.2% |

The unsigned7 build median rises by about 0.2425ms (21.1%). Its per-case build ratios range 1.131–1.268. The memory increase is a real implementation tradeoff, even when operation queries are faster. `memory_usage()` is allocated data-structure storage reported by each API, not process RSS.

Only six operation rows had every sample at least 50ms: signed64/unsigned64 random updates, inserts and erases. Their ratios are descriptive longer-duration controls; the full raw samples and ranges remain in `results.json`.

The initial harness compile failures were `-Wmisleading-indentation` warnings in the inherited compact benchmark driver; adjacent statements were split onto lines. Exact flags and checks were retained. Diagnostics are in `compile.log`, explanation in `initial-harness-failure.txt`. No benchmark samples were discarded.

Reproduction: the measured runner has fixed original paths and hashes; its complete commands, source/compiler/binary/header hashes and hardware details are in `manifest.json`. A separate reusable driver at `../run_broad_api_controls.py` accepts `--repo`, required fresh `--work` and required `--header-hashes`. It has not been benchmarked against any later revision.
