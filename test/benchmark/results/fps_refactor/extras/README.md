# FPS additional operation refactoring

Baseline: `32f6930`. The patches in this directory modify only composition and evaluation internals; public APIs and algorithms are unchanged. Column SIMD replaces four assembly routines with two template instantiations. Evaluation shares the small product tree build and descent between evaluation and interpolation. `sz` is also renamed `scale_z` to avoid a solver macro collision.

The isolated source snapshots were `/workspace/cp-fps-256-extras-study/before` and `candidate`. Both benchmark programs use GCC with the flags in `metadata.json`; all timings hold both recorded exclusive locks. Composition uses `test/benchmark/fps_tuning_advanced.cpp` and evaluation uses `test/benchmark/fps_split.cpp`. Benchmark commands, all samples and binary hashes are retained in the Python and JSON files. Run the scripts with a directory containing `before_advanced`, `candidate_advanced`, `before_split` and `candidate_split`.

To reproduce snapshots, extract `git archive 32f6930`, copy it to `candidate`, and apply both patches there. Build the benchmark source in each snapshot with its include directory. Build `column_primitive.cpp` against the **baseline** include directory: the candidate kernel is embedded under a distinct namespace, so each output word is compared directly to the assembly output. This covers both even and wider columns. The evaluation checksum checks use dense random coefficients and distinct sequential points at the 36 sizes recorded in `metadata.json`.

The evaluation benchmark binary precedes the local `sz` rename; that rename changes no generated arithmetic. Final combined validation is recorded separately by the main refactoring task.
