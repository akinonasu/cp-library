# Sieve / bitwise transform optimization validation

2026-10-17. Baseline implementation: `0a96edcbfda903acd7dcadd0e788be263c9589cf`.
The final changes were integrated on `1bb6151acbc2bf863c116009028dc46f6e977a55`; upstream prefix-minimum and link-cut-tree changes were preserved. None changed the dependencies of the affected tests. The updated standalone integration test also passed with all three modified headers force-included.

## Measurements

- [Prime sieve](prime_sieve.md): segmented odd sieve, periodic pre-sieve, word-wise enumeration.
- [Bitwise transforms](bitwise_transforms.md): radix-4 stage fusion, cache blocking, scoped GCC optimization settings.
- [Environment](results/portable_optimization/environment.json): local GCC 14.2.0, Linux x86_64, Intel Xeon Platinum 8573C. These are not AtCoder judge measurements.

AtCoder's [official language list](https://img.atcoder.jp/file/language-update/2025-10/language-list.html) specifies GCC 15.2.0 and `-O2 -march=native -std=gnu++23 -ftrivial-auto-var-init=zero` among the current C++ flags. The [June 2026 notice](https://atcoder.jp/posts/language_update20260616_en) confirms the compiler update. No unverified judge CPU model is assumed. The benchmark reports retain AC/reference implementation links and comparison limitations.

## Local validation

Seven modified/transitively dependent headers passed standalone double-include compilation with `-Werror`. Five affected unit suites passed GCC and ASan/UBSan:

- `prime_sieve.cpp`
- `components_number_theory.cpp`
- `bitwise_transforms.cpp`
- `transforms.cpp`
- `convolution.cpp`

The focused sieve extended mode additionally passed full boundary/random checks and known prime counts through 500,000,001. Independent review tested the sieve byte fallback and transforms with the scoped GCC pragmas removed. Floating results are compared bit-for-bit; narrow integers, non-copyable/non-default-constructible/custom operation types and convolution integration are covered.

Reproduce affected checks:

```sh
python test/verify.py --unit-only --cxx g++ --selection test/benchmark/results/portable_optimization/selection.json
ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1 python test/verify.py --unit-only --cxx g++ --sanitize --selection test/benchmark/results/portable_optimization/selection.json
```

The sandbox's LeakSanitizer exits with a ptrace incompatibility, so only leak detection was disabled for the successful sanitizer run. ASan and UBSan remained enabled. Clang, Apple Clang/macOS and online judge cases were not run locally; the repository's existing Linux verification and macOS portability workflows provide those checks after publication. No new architecture-specific instruction requirement is introduced.

[Successful sanitizer log](results/portable_optimization/all-sanitize.log), [sieve GCC log](results/portable_optimization/sieve-gcc.log), [final default sieve test](results/portable_optimization/sieve-default-gcc.log), [transform GCC log](results/portable_optimization/transforms-gcc.log), [latest-main integration](results/portable_optimization/latest-main-integration.log).

## Documentation-only base update

Before publication, main advanced to `1d9f35211be50d98b66d42289e620edc78082636` with documentation changes only. All upstream documentation was preserved, including the new `prime_sieve(n)` API table. The optimized sieve description and complexity were merged below that table. The API example passed a focused syntax check; production code and test dependencies were unchanged, so previous test and timing results remain applicable.
