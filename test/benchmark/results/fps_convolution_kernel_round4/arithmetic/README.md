# Product arithmetic candidates, round 4

No candidate was adopted. Production headers were not edited. The fixed baseline is `7521ea8703a362aaf3ccd370188fccab86050848`; the unedited reference is Library Checker submission [409465](https://judge.yosupo.jp/submission/409465).

All builds used GCC 14.2 and the flags in `probe.py`: `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`. Builds held shared `/tmp/cp-fps-tuning.lock`; whole-process timings held it exclusively and pinned CPU 2. Each row has one warmup and nine measured samples per variant, with deterministic shuffled order. Official checker arguments were `input actual answer`. Official input, answer and checker hashes, complete commands, actual source/header hashes and every sample are retained in the five raw JSON files. All 540 outputs passed. All recorded source/binary/tree hashes were unchanged at the end of each experiment. No 21-trial confirmation was run because no candidate passed the initial whole-process screen.

| candidate | input | baseline ms | candidate ms | 409465 ms |
|---|---|---:|---:|---:|
| tile64 | max_random_00 | 11.808836 | 11.965639 | 9.728314 |
| tile1024 | max_random_00 | 11.808836 | 11.927933 | 9.728314 |
| tile64 | max_random_01 | 11.877316 | 11.969104 | 9.693742 |
| tile1024 | max_random_01 | 11.877316 | 11.947782 | 9.693742 |
| tile64 | all_same_00 | 12.737779 | 13.118028 | 8.328648 |
| tile1024 | all_same_00 | 12.737779 | 12.760143 | 8.328648 |
| pairall | max_random_00 | 11.951108 | 12.079387 | 9.678859 |
| staticlevels | max_random_00 | 11.951108 | 11.929056 | 9.678859 |
| top32 | max_random_00 | 11.951108 | 12.849057 | 9.678859 |
| pairall | max_random_01 | 12.040190 | 12.304102 | 9.678568 |
| staticlevels | max_random_01 | 12.040190 | 12.321479 | 9.678568 |
| top32 | max_random_01 | 12.040190 | 12.570491 | 9.678568 |
| pairall | all_same_00 | 12.860503 | 12.898298 | 8.268567 |
| staticlevels | all_same_00 | 12.860503 | 12.932920 | 8.268567 |
| top32 | all_same_00 | 12.860503 | 13.180399 | 8.268567 |
| leaf32 | max_random_00 | 12.603630 | 13.522821 | 10.830557 |
| leaf32 | max_random_01 | 12.286458 | 12.980082 | 10.221008 |
| leaf32 | all_same_00 | 13.979580 | 13.441057 | 8.576626 |
| leaf32asm | max_random_00 | 12.738601 | 12.976525 | 9.897846 |
| leaf32asm | max_random_01 | 12.909729 | 13.254941 | 10.957757 |
| leaf32asm | all_same_00 | 13.243484 | 13.728247 | 8.455276 |
| karatsuba4 | max_random_00 | 12.198296 | 13.262312 | 9.950765 |
| karatsuba4 | max_random_01 | 12.301119 | 13.806273 | 9.944576 |
| karatsuba4 | all_same_00 | 12.802086 | 13.891199 | 8.505089 |

`tile64` and `tile1024` change the largest cached subtree. `staticlevels` makes each tile stage a template constant. `top32` interleaves both factors in the first radix-8 with AVX512VL register availability. `pairall` repeats an already rejected round-3 experiment; it was identified after this screen and is preserved as redundant evidence, not a new result.

`leaf32` retains four canonical/weighted factors and coefficients, forms cyclic windows with `vpermt2d`, and performs the existing exact 64-bit sums using `vpmuludq`. It uses the preceding even window for the next odd sum. The compiler used all 32 vector registers but also emitted 105 stack-reference lines. `leaf32asm` fixes these four products to 32 vector registers and returns to the existing inverse h=1 helper. Both lose the original bottom kernel overlap with the next forward group; neither improved the random inputs.

`karatsuba4` splits each degree-8 leaf into even/odd degree-4 polynomials modulo `Y^4-w`. Three degree-4 products reduce scalar products from 64 to 48. It reconstructs even coefficients as `C0+Y*C2` and odd coefficients as `Cs-C0-C2`. Packing, cyclic permutations and weight correction outweighed the reduced multiplication count. The experiments establish the behavior of these prototypes only; they do not rule out other scheduling or arithmetic improvements.

The AVX512VL candidates retain the AVX2 fallback behind runtime feature checks. They add no public input alignment, padding or mutation requirement. The extra targets were exercised only on this AVX512-capable AMD EPYC 9V74 host. These rejected prototypes were checked on the three listed official inputs, not certified for adoption with a full API/sanitizer suite.

Replay into a new directory without altering the retained snapshots:

```sh
python3 replay.py --repo /workspace/cp-library --work /workspace/cp-fps-round4/kernel-replay
```

The replay validates archived file hashes, reconstructs the exact baseline dependency files from Git, overlays each saved actual header, and uses the frozen original reference source. It records the new absolute commands and samples. Different source paths, compiler versions, machine state and allocator addresses can change binary hashes or timings. The original commands and original hashes remain in the raw files.
