# CSR code generation screening

These are exploratory measurements, not the final library benchmark. Sources were frozen from `bd29f70` (`old`) and `c075fb7` (`current`) before compiling. No live library source was edited.

The full header copies and generated `.s` files are omitted from this archive. `source_manifest.json` records all frozen C++ file hashes and exact Git revisions. Each candidate patch is relative to `c075fb7`. `replay.py` reconstructs all 9 trees in an ignored directory and audits them against the raw measurement hashes; [reconstruction_audit.json](reconstruction_audit.json) records the successful audit. Drivers are saved as `phase.cpp` and `actual.cpp`.

```sh
python3 test/benchmark/results/shortest_path_round5_variants/codegen/replay.py
```

The resulting default work directory is `.verify-helper/cache/shortest-path/round5/codegen-replay`. `--destination /tmp/cp-round5-codegen-replay` selects another location. The archived candidate preparation scripts also accept this directory through `CP_SP_ROUND5_CODEGEN_WORK`; `prepare.py` pins the original revisions rather than reading live library sources.

The following commands are optional reproductions of the screening and assembly analysis. They were not rerun when archiving.

```sh
CP_SP_ROUND5_CODEGEN_WORK=.verify-helper/cache/shortest-path/round5/codegen-replay \
  python3 test/benchmark/results/shortest_path_round5_variants/codegen/run.py
python3 test/benchmark/results/shortest_path_round5_variants/codegen/regenerate_asm.py \
  --work .verify-helper/cache/shortest-path/round5/codegen-replay
python3 test/benchmark/results/shortest_path_round5_variants/codegen/asm_compare.py \
  --work .verify-helper/cache/shortest-path/round5/codegen-replay
```

`run.py --variants current pointer pointer_all --trials 17 --output pointer_native.json` and `run.py --variants current destination destination_local --trials 17 --output destination_native.json` select later candidate groups; the archived raw files also include `old` for the pointer comparison. The uninstrumented/phase variants and default 8 cases are defined in the runner. [environment.json](environment.json) saves the host details collected during this investigation, while each raw JSON saves its compiler, flags, CPU and trial count.

GCC 15.2 / WSL2 Linux x86_64, CPU 2, flags `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=native`. Compile and measurement share exclusive `/tmp/cp-wm-opt.lock`. Input, binaries and output use Linux `/tmp`; each trial launches a fresh process, randomizes variant order and includes process startup/exit in `whole_ms`. There is one warmup, whose output path is checked by the official checker. Every trial compares the distance.

`phase.cpp` includes clocks; `actual.cpp` is the uninstrumented library submission. Do not compare their whole times as interchangeable measurements. Source and input SHA256 values, raw trials and medians are in each JSON file.

## Candidates

`prepare.py` creates the baseline and the first three candidates:

- `skip`: do not count active outgoing rows for fewer than 8192 vertices; compute the same prefix sum with the original loop instead.
- `split`: move the prefetch scatter loop into a separate private member. GCC still inlines this helper into `finish`.
- `split_skip`: both changes.

`screen_native.json` has 8 representative cases and 9 trials. None of these candidates improves the dense cases consistently, so they were rejected. In `max_dense_random_01`, actual medians were current 6.292 ms, skip 6.333 ms, split 6.498 ms and split_skip 6.395 ms.

`prepare_pointer.py` adds Arc pointer cursors, retaining size_t offsets, the public API, generic weights and input edge order:

- `pointer`: use them only for fewer than 8192 vertices and at least 16 edges per vertex.
- `pointer_all`: use them for all nonempty general layouts.

`pointer_native.json` has the same 8 cases and 17 trials. Dense 01 was current 6.261 ms and pointer 6.290 ms. Dense 00 and zero also did not improve. Unconditional pointer cursors additionally slowed sparse and star cases. Both were rejected.

`prepare_destination.py` computes the destination reference before constructing/assigning the Arc:

- `destination`: `auto &destination = arcs_[--offset_[edge.from]]; destination = {edge.to, edge.weight};`.
- `destination_local`: create a local const Arc and copy-assign it after computing the destination.

`destination_native.json` has the same 8 cases and 17 trials. Dense 01 was current 6.082 ms and destination 5.938 ms, but sparse 01 was 13.527 to 14.024 ms and swirl was 15.313 to 15.543 ms. The generated scatter loop still uses separate 32-bit field loads/stores; both candidates produce the same loop. The rewrite changes load ordering, and this experiment does not establish a useful general improvement. Both were rejected.

## Assembly comparison

Compile each frozen `actual.cpp` with the flags above and `-S`, then run `asm_compare.py`. `assembly_comparison.json` and `assembly_hashes.json` compare old and current function bodies after replacing compiler-generated local labels consistently within each body.

Only `DirectedCSR<uint32_t>::finish` differs. The other 29 functions, including main, bulk input, output, Dijkstra path/run and unique-path handling, are identical after this normalization. This rejects a parser instruction/register allocation regression for this translation unit. Function addresses and code layout still differ, so this comparison alone does not reject all code-placement effects. The 707-row active count and inactive prefetch branch remain differences in the affected finish function; their presence alone does not explain the previous 0.282 ms whole-program difference.

`copy_probe.cpp` is a small compiler-only experiment. Isolated assignment from Edge to Arc can use one 64-bit load/store, but the actual scatter loop does not. No object-representation assumptions, memcpy specializations or type restrictions were added to the library.
