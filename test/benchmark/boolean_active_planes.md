# Boolean convolution: inactive planes

The twelve wrappers transform only coefficient-bit planes supported by both inputs. Exact saturation ends the scans early. AND reductions use width-truncated complements; `bool` uses one bit; GCD/LCM exclude index zero. Padding is not scanned. A plane with an all-zero indicator on either side has zero bilinear convolution, so skipping it preserves the neutral output bit.

Public signatures, integral types, output lengths, empty/signed behavior, index-zero results and arithmetic transforms are unchanged. No ISA or platform specialization is added. With width `W` and `k` active planes, the change replaces `W` plane convolutions with `k`, adding at most linear input scans. Full-width support can incur overhead without eliminating work.

## Source and method

Baseline: `188eb0c37e607e74ee613b493a5833f89e870844`. Candidate SHA-256:

- `convolution/boolean_convolution.hpp`: `d4892349ca7c413a6dedcb763357ef84d3dc20823319f9386952e2d1b2999a5e`.
- `test/unit/convolution.cpp`: `8c2fdc97b70adb30773e4894db289669cff4e7f8d2029986c2c6bf274a8d869d`.

The seven included project headers are identical between arms. [Run 39273459327](https://github.com/qandori/cp-library/actions/runs/39273459327), 2026-10-19, launch `23efd2a27a4ab4b4cffb12fccddc15e869fe84d9`: Ubuntu 26.04.1 x86_64, CPU affinity 0, GCC 15.2.0. Both arms use separate translation units, external calls and no LTO:

    -std=c++23 -O2 -Wall -Wextra -Werror -fno-lto
    -march=x86-64 -mtune=generic -mno-avx -mno-avx2
    -mno-avx512f -mno-bmi -mno-bmi2

Assertions and existing header optimization pragmas remain enabled. The fixed [protocol](https://github.com/qandori/cp-library/blob/23efd2a27a4ab4b4cffb12fccddc15e869fe84d9/test/benchmark/boolean_active_planes/protocol-v2.md) and [runner supplement](https://github.com/qandori/cp-library/blob/23efd2a27a4ab4b4cffb12fccddc15e869fe84d9/test/benchmark/boolean_active_planes/REMOTE-PROTOCOL.md) define all 18 cells, witnesses, caps and gates. No experiment runner or raw logs are added to product source or normal CI.

This is **whole-wrapper warm-call throughput**, not cold latency. Timing includes scans, wrapper allocations, transforms, reconstruction, internal-buffer destruction and result move-assignment. Inputs and outer result slots are prepared beforehand. Returned vectors remain alive in distinct slots until timing stops; complete output comparison, hashing and caller-side destruction follow outside timing.

Seeds: 857123/857124/857125. Baseline-only calibration reaches >=100 ms per seed, then freezes common counts. Each of nine pairs cycles seeds and alternates AB/BA; each timed batch has two same-seed/build/count warmups. No observations are dropped or candidate-conditioned recalibration performed. The campaign completed in 294.582 seconds, including 108 full-size quadratic-oracle preparations, calibration, warmups and 234 pairs (162 A/B, 72 A/A).

## All fixed cells

Types `u64/u32/i64` mean `uint64_t/uint32_t/int64_t`; operation labels omit `_convolution`. Times are median microseconds per complete call. B/A is the median of nine **paired** ratios, not the quotient of displayed time medians. Lower is better.

| Cell | Operation / distribution | Type, lengths | Active planes | Baseline us | Candidate us | Paired B/A |
| --- | --- | --- | ---: | ---: | ---: | ---: |
| 1 | xor_or, low 8 bits | u64, 1024/1024 | 8 | 1516.147 | 194.851 | 0.128479 |
| 2 | gcd_or, low 8 bits; bit 63 only at index 0 | u64, 1024/1024 | 8 | 540.125 | 73.282 | 0.135765 |
| 3 | subset_and, complements of low 8 bits | u64, 256/256 | 8 | 1163.480 | 148.930 | 0.128017 |
| 4 | and_or, bit 63 only | u64, 1024/1024 | 1 | 651.411 | 15.400 | 0.023671 |
| 5 | or_or, disjoint 0x55 / 0xaa support | u64, 1024/1024 | 0 | 699.448 | 4.422 | 0.006326 |
| 6 | lcm_and, complements of low 8 bits | u64, 1024/342 | 8 | 504.593 | 68.066 | 0.134912 |
| 7 | lcm_and, cell 6 inputs exchanged | u64, 342/1024 | 8 | 504.536 | 67.778 | 0.134327 |
| 8 | gcd_and, sole coefficient 0 | u64, 1/1 | 0 | 1.705 | 0.029 | 0.016744 |
| 9 | xor_or, full-width random | u64, 1024/1024 | 64 | 1507.552 | 1515.131 | 1.005027 |
| 10 | subset_or, full-width random | u64, 256/256 | 64 | 1161.862 | 1161.751 | 1.000088 |
| 11 | xor_and, positive low 8 bits | u64, 1024/1024 | 64 | 1513.074 | 1520.671 | 1.004684 |
| 12 | or_or, mixed bool | bool, 1024/1024 | 1 | 11.726 | 11.704 | 0.997499 |
| 13 | xor_or, full-width support only at last coefficient | u64, 1024/1024 | 64 | 1516.967 | 1526.523 | 1.006023 |
| 14 | and_and, tiny all-zero inputs | u64, 3/1 | 64 | 2.860 | 2.892 | 1.011034 |
| 15 | and_or, full-width uint32 | u32, 1024/1024 | 32 | 323.427 | 323.474 | 0.999407 |
| 16 | xor_or, full-width signed patterns | i64, 1024/1024 | 64 | 1509.859 | 1519.104 | 1.006237 |
| 17 | or_and, all-one coefficients | u64, 1024/1024 | 0 | 704.977 | 0.755 | 0.001073 |
| 18 | xor_or, all-false bool | bool, 1024/1024 | 0 | 24.159 | 0.633 | 0.026196 |

Primary cells 1–3 passed B/A <= 1/1.10 with 9/9 faster pairs each: reciprocal paired ratios 7.7834x, 7.3656x and 7.8115x. These gains concern the specified low-active-plane inputs only.

All eight no-elimination controls, 9–16, passed median B/A <= 1.03 and at least 7/9 ratios <= 1.05; all qualified 9/9. The worst median was tiny cell 14, about 1.10% slower. Small differences near 1.0 are not claimed as meaningful speedups.

Beginning and ending A/A guards used the unchanged baseline on cells 9/10/12/14. All eight passed median [1/1.03, 1.03], with 9/9 individual ratios within [1/1.05, 1.05]. Cells 4–8 and 17–18 are diagnostic. Cell 17's candidate batches were only 0.190–0.205 ms: its large ratio is limited evidence, not a precise speed claim. Cell 8 retains the tiny-call case; 6/7 retain both input orders.

Exact raw-log reconstruction and arbitrary-precision audit reproduced every gate and runner provenance hash. Audited result SHA-256: `400084a45f1dfa39ae8531e75b7e3e67611b23d5375d90a008bc614639bdbb83`. Full evidence remains outside product source.

## Failures, correctness and limits

The local screen was inconclusive: unstable initial A/A observations preceded the two-second batch cap; no candidate A/B batch ran. [First remote attempt 39270609963](https://github.com/qandori/cp-library/actions/runs/39270609963) failed backend-path preflight before compilation/timing. The corrected public-runner experiment above is separate, not pooled with or resumed from either failure. No candidate timing selected code or calibration counts; counts never changed after baseline calibration.

Local qualification passed the final convolution unit with GCC 14.2/15.2, standalone-header warnings-as-errors, and 93,372 external public-wrapper/oracle cases. GCC 14.2 ASan/UBSan separately passed those cases with `detect_leaks=0` for the ptrace restriction; this is not a leak-check pass. Compact permanent tests cover empty/unequal inputs, zero/disjoint support, bit 63, index-zero-only support, complements, bool and narrow signed/unsigned types. No large optional corpus enters CI.

Mac/Clang were not run locally. Source-level compatibility reasoning and benchmark success do not establish product CI. Both reverse-dependent units (`convolution.cpp`, `subset_convolution.cpp`), standalone/bundle checks, sanitizers and unchanged Apple Clang CI must qualify the latest product head before merge.

This is an implementation-derived zero-plane optimization, measured against the library baseline in these workloads only. No current-fastest submission was consulted. Historical arithmetic-engine notes in [subset_convolution.md](subset_convolution.md) are context, not competitor measurements for these wrappers. There is no Library Checker fastest, end-to-end judge, cold-latency, universal, native-ISA or Mac-speed claim.
