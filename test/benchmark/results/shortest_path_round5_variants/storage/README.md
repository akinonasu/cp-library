# CSR overwrite-storage screening

This candidate was rejected after the broader 41-case evaluation. No overwrite storage was added to the production library. See [round 5](../../../shortest_path_round5.md) for the formal results and decision.

This archive records a general storage prototype relative to `c075fb7`, before the final candidate's empty-allocator layout and zero-size copy guard changes. It does not depend on a particular edge order or graph density. The final candidate's measurements and validation are recorded separately in the main round 5 report.

The former general CSR constructor resized `vector<pair<int,T>>`, which initializes each Arc, and then overwrote every Arc during scatter. The prototype allocates an Arc array with `std::allocator` and begins each Arc lifetime with `std::construct_at` at its final position. It keeps size_t offsets, public Arc/API types, input edge order, existing prefetch dispatch and single-edge handling.

The raw array is selected only for trivial weight types, trivially destructible Arc, nothrow Arc construction from `(int, const T&)` and nothrow Arc copy construction. Other types retain vector storage. Copy uses `uninitialized_copy_n`, moves transfer pointer/size/capacity together, clear retains the allocation, and deallocation uses the original capacity. No vector size manipulation, pair representation assumptions or uninitialized-object reads are used. Prefetch addresses are formed from `data()+index`.

## Conditions

GCC 15.2 / WSL2 Linux x86_64, CPU 2, exclusive `/tmp/cp-wm-opt.lock`. Both original and shuffled inputs use fresh processes, one warmup and 17 measured trials with randomized execution order. Input, executable and output use Linux `/tmp`. Flags are `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero`; native adds `-march=native`. [environment.json](environment.json) records the host and problem revision. Each raw JSON includes compiler, flags, affinity, source hashes, every sample and medians.

`phase.cpp` includes clocks around input, CSR finish, path computation and output. `actual.cpp` is the uninstrumented submission; its whole-program time includes process startup/exit. These are different programs and their whole times are not interchangeable. Every trial compared distances; each warmup path passed the official checker.

## Preliminary results

Actual submission medians in ms:

| Case | native before | native prototype | generic before | generic prototype |
| --- | ---: | ---: | ---: | ---: |
| dense random 00 | 5.968 | 5.748 | 6.531 | 6.353 |
| dense random 01 | 5.987 | 5.757 | 6.854 | 6.458 |
| dense zero 00 | 5.615 | 5.488 | 6.007 | 5.984 |
| dense long 00 | 6.337 | 6.184 | 6.810 | 6.857 |
| sparse random 01 | 13.436 | 13.385 | 13.442 | 13.764 |
| star 00 | 8.996 | 8.987 | 9.811 | 9.316 |
| line 00 | 22.270 | 21.706 | 21.908 | 22.307 |
| grid swirl 00 | 14.482 | 14.367 | 15.325 | 15.282 |

[Native samples](screen_native.json) and [generic samples](screen_generic.json) include small increases as well as improvements. These trials do not establish that every input improves.

For the next experiment, only edge order was shuffled with seed `20261019`; vertices, edges, weights, source, target and correct distance stayed the same. [prepare_inputs.py](prepare_inputs.py) and [shuffled_inputs.json](shuffled_inputs.json) define the transformation and source/output hashes. [Shuffled native samples](shuffled_native.json) retain all trials.

| Shuffled case | native before | native prototype |
| --- | ---: | ---: |
| dense random 00 | 6.373 | 6.931 |
| dense random 01 | 6.423 | 6.230 |
| sparse random 01 | 12.753 | 12.529 |
| star 00 | 8.723 | 8.466 |
| line 00 | 21.810 | 21.519 |
| grid random 00 | 8.250 | 7.882 |

Shuffled dense 00 increased by 8.8% in the actual-submission median. Its phase-program whole medians were 6.924 to 6.912 ms, and build medians were 2.185 to 2.158 ms. The whole-time samples split into groups around 6.1 and 7.0 ms. This experiment does not isolate a fixed storage regression; the increase remains in the archive and is not omitted from the evaluation. The final library is evaluated in a separate, broader run.

## Reconstruction

Only the two drivers, patches and hashes are archived for this screening; full header trees and large input copies are omitted. [source_manifest.json](source_manifest.json) pins the baseline revision and every frozen C++ source. [reconstruction_audit.json](reconstruction_audit.json) verifies that the patch reconstructs both trees exactly, all three measurement archives use the same source hashes, trial counts match, and shuffled input hashes match the generator record.

```sh
python3 test/benchmark/results/shortest_path_round5_variants/storage/replay.py
```

The default reconstructed directory is `.verify-helper/cache/shortest-path/round5/storage-replay`; `--destination` can select another directory. The following optional commands reproduce the screening. They were not run again when archiving.

```sh
CP_SP_ROUND5_STORAGE_WORK=.verify-helper/cache/shortest-path/round5/storage-replay \
  python3 test/benchmark/results/shortest_path_round5_variants/storage/run.py \
  --variants current overwrite --trials 17 --output screen_native.json
CP_SP_ROUND5_STORAGE_WORK=.verify-helper/cache/shortest-path/round5/storage-replay \
  python3 test/benchmark/results/shortest_path_round5_variants/storage/run.py \
  --variants current overwrite --trials 17 --generic --output screen_generic.json
CP_SP_ROUND5_STORAGE_WORK=.verify-helper/cache/shortest-path/round5/storage-replay \
  python3 test/benchmark/results/shortest_path_round5_variants/storage/prepare_inputs.py
CP_SP_ROUND5_STORAGE_WORK=.verify-helper/cache/shortest-path/round5/storage-replay \
  python3 test/benchmark/results/shortest_path_round5_variants/storage/run.py \
  --problem .verify-helper/cache/shortest-path/round5/storage-replay/shuffled \
  --variants current overwrite --trials 17 --output shuffled_native.json \
  --cases max_dense_random_00_shuffled max_dense_random_01_shuffled \
  max_sparse_random_01_shuffled max_star_00_shuffled line_00_shuffled grid_random_00_shuffled
```

## Formal candidate archive

The broader measurements are in `shortest_path_round5_native` and `shortest_path_round5_generic`. Their `sources/current` directories contain the exact final candidate headers, including `OverwriteArray`. These sources differ from the preliminary patch by the empty-allocator layout and zero-size copy guards.

`formal_validation.json` and `formal_validation.log` record 14 GCC/Clang unit checks, 8 sanitizer checks, and 174 official checker runs for this rejected candidate. `formal_docs.log` records its documentation check. `overwrite_array_unit.cpp` and `overwrite_array_doc.txt` preserve the removed candidate's unit and usage contract. The aligned/throwing-weight CSR unit remains in the production test suite.

`measured_check.py`, `measured_docs.py`, and `measured_summary.py` preserve the scripts as measured, for hash auditing. They retain their original repository-relative path assumptions and are not entry points at this archive location. The current round 5 checker validates only the retained CSR unit against the unchanged production header.
