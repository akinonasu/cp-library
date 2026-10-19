# Twiddle constants in additional YMM registers

This final arithmetic probe was rejected. It preserves the original forward-pair and bottom-kernel scheduling. The three stack twiddle vectors used by `forward_pair` move to YMM16–18; the original sixteen scratch registers are untouched. Three setup stores and eight L1 vector memory operands per loop iteration are removed. The existing call uses h=4, removing 32 such vector reads per call.

The fixed baseline is `7521ea8703a362aaf3ccd370188fccab86050848`. All three variants retain the same source apart from the recorded patches. `vlcontext` changes only the private convolution header target from AVX2 to AVX2+AVX512F+AVX512VL. `vlregs` adds the three register replacements. `audit.json` records that undoing those changes restores the original forward-pair byte for byte. `bottom::both`, all leaf arithmetic, public input/output and the original instruction order are otherwise retained.

These globally retargeted private headers were run only on the AVX512-capable AMD EPYC 9V74 host. They do not provide CPUID dispatch and must not replace production headers. The production library was not edited. The earlier frozen 35-file archive remains unchanged.

GCC 14.2, the exact original flags and source/binary/header hashes are in `twiddle32_9.json`. Shared flock covered builds; exclusive `/tmp/cp-fps-tuning.lock` and CPU 2 covered timings. Each input has one warmup and nine measured samples per variant, deterministically shuffled. All 80 outputs passed official checker with argument order input/actual/answer. Source, binary and tree hashes were unchanged after the run.

| official input | AVX2 baseline ms | VL context ms | VL registers ms | 409465 ms |
|---|---:|---:|---:|---:|
| max_random_00 | 12.089415 | 12.176935 | 12.247960 | 10.037545 |
| max_random_01 | 12.258845 | 12.246117 | 12.168073 | 9.790856 |

The paired registers-minus-context medians are max_random_00: +0.200890 ms (3/9 wins), max_random_01: +0.028302 ms (4/9 wins). There is no stable whole-process benefit, so no 21-trial confirmation or adoption checks were run. This result concerns this exact constant-register substitution and does not rule out other arithmetic scheduling.

Replay into a new directory:

```sh
python3 replay.py --repo /workspace/cp-library --work /workspace/cp-fps-round4/twiddle-replay --experiments twiddle32_9
```

The replay reconstructs baseline dependencies from Git, overlays each saved actual header and uses the unedited retained 409465 source. It writes new absolute commands and measurements without replacing the retained raw evidence.
