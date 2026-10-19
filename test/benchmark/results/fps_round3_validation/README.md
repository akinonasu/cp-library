# Round 3 validation

**The authoritative combined gate is [scope-audit.json](scope-audit.json), with `status: passed`.** It requires all compiler/test commands to succeed, exact frozen-scope log coverage, current dependency hashes, and the bounded pragma equivalence evidence described below.

The strict historical runner result is preserved unchanged as [result.json](result.json), with `status: failed`, `dependency_hashes_match: false`, and `plan_fingerprints_match: false`. Its compiler/test commands all returned zero. This raw failure records one source spelling change during the run; it has not been silently converted into a clean source-hash run.

| Check | Required | Executed | Result |
| --- | ---: | ---: | --- |
| GCC 14 standalone headers | 43 | 43 | passed |
| GCC 14 unit tests | 43 | 43 | passed |
| Clang 19 standalone headers | 43 | 43 | passed |
| Clang 19 unit tests | 43 | 43 | passed |
| GCC 14 ASan/UBSan native-focused units | 5 | 5 | passed |

The final scope is the dependency/fingerprint difference from `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`, calculated by the existing `.verify-helper/ci.py`. The Mint constructor attribute broadens it beyond FPS to matrix, combinatorial, transform, and tree-composition tests. The [selection.json](selection.json) SHA-256 is `b2e965770fdc2e3709af7ff4cab6356f5b06e600997fc83d465a1e681bfc3e37`: headers 43, units 43, native-focused sanitizer units 5, judge drivers 53, bundles 43. Judge and bundle validation are separate artifacts in this directory; they are not asserted by the unit gate.

The five sanitizer units are `fps.cpp`, `fps_io.cpp`, `fps_exp_tail.cpp`, `fps_bostan.cpp`, and `fps_convolution_borrow.cpp`, under `test/unit/`. Selection uses their intersection with all selected units, including units whose body is unchanged but whose dependencies changed.

GCC and Clang dependency scans cover all 43 selected headers and 43 selected units: 172 `-MM` commands. The union contains 188 actual source/control files, including 22 controls: the runner, `test/verify.py`, `.verify-helper/ci.py`, and workflow files. Compile/test subprocesses use shared `/tmp/cp-fps-tuning.lock`; work files and compiler temporary files use `/workspace/cp-fps-scratch-round3/validation`. Exact tools, executable hashes, flags, environment, timing, and commands are in [conditions.json](conditions.json) and [result.json](result.json). Logs are [GCC](gcc14.log), [Clang](clang19.log), and [sanitizer](gcc14-sanitize.log).

## One documented normalization

During validation, the root agent replaced `#pragma GCC target("avx2")` in `components/fps_bostan.hpp` with the equivalent `_Pragma("GCC target(\"avx2\")")` spelling, so the documentation bundler would preserve the target directive. This is the only allowed source difference:

- Original SHA-256: `bbe4528ee226bcf0ea90d143f0b5832ef6f69b2162b448ec845d249349bc7be3`.
- Canonical SHA-256: `bda7e2f7b2189023cc5da6b695eddc5bcde4ff81eb8407d933c0cf18fb13e258`.

[dependencies-before.json](dependencies-before.json) and [dependencies-after.json](dependencies-after.json) retain that difference. All other 187 entries, including all 22 controls, are identical. Every one of the 188 current files matches the after snapshot.

The separate final audit requires the old/new file hashes to match exactly, binds the [preprocessor evidence](docs_pragma_preprocessing.json) to those hashes, and checks its identical normalized `-E -P` output. It also checks the [same-path object evidence](docs_pragma_codegen.json), whose two objects are byte-identical. It derives the original header bytes by reversing this one directive replacement and verifies the original hash again. It temporarily substitutes those bytes only inside Python's read-only CI-plan calculation; it does not edit the header. The restored-content plan must equal every fixed selection fingerprint, while the current plan must equal [after-plan.json](after-plan.json), with the same selected target lists.

The compiler/test success conditions remain mandatory in the normalization branch. The audit compares every ordered header/unit marker in each log with the frozen [scope contract](scope-contract.json), requires 86 GCC markers, 86 Clang markers, five sanitizer markers, zero return codes, matching log hashes, and final `Verification passed` lines. The fixed selection SHA binds the whole file, preventing accidental scope reduction. Thus the authoritative result combines actual full-scope tests and a strictly bounded source-equivalence check, while the raw runner's source-change failure remains visible.

## Reproduction

Repeat compilation/tests on the canonical snapshot in a new output directory, without overwriting these historical records:

```sh
python3 test/benchmark/fps_round3_validation.py --execute \
  --output /workspace/cp-fps-scratch-round3/new-validation \
  --work /workspace/cp-fps-scratch-round3/new-validation-work
```

To recheck the saved combined gate while the current files still match its after snapshot, run `flock -s /tmp/cp-fps-tuning.lock python3 test/benchmark/fps_round3_validation_audit.py`.

The runner by itself reports its strict source-hash result. A fresh run wholly on the canonical header can use a newly bound selection contract; the saved audit is intentionally tied to this historical selection and its single normalization. Do not relabel the saved historical result to describe a different run.
