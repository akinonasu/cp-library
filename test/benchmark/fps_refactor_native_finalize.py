import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / 'test/benchmark/results/fps_refactor'
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
load = lambda name: json.loads((RESULT / name).read_text())
main = load('validation.json')
initial = load('native_sanitize_initial.json')
warning = load('native_evaluation_warning.json')
evaluation = load('native_evaluation_sanitized.json')
advanced = load('native_tuning_advanced.json')
assert main['completed'] and evaluation['completed'] and advanced['completed']
for data in [initial, warning, evaluation, advanced]:
    assert data['source_sha256'] == main['source_sha256']
assert initial['baseline_commit'] == main['baseline_commit']
assert warning['baseline_same_warning']
assert [row['returncode'] for row in warning['checks']] == [1, 1]
assert initial['script_sha256'] == sha(RESULT / 'native_sanitize_initial_runner.py')
assert evaluation['script_sha256'] == sha(RESULT / 'native_sanitize_evaluation_runner.py')
assert initial['compiler_sha256'] == evaluation['compiler_sha256'] == advanced['compiler_sha256'] == main['compiler_sha256']['g++']
assert evaluation['checks'][:2] == initial['checks'][:2]
checks = [row for row in initial['checks'] if row['returncode'] == 0]
assert len(checks) == 10
advanced_compile, advanced_run = advanced['checks']
advanced_compile = dict(advanced_compile, source=advanced['source'], binary_sha256=advanced['binary_sha256'])
checks += evaluation['checks'][2:] + [advanced_compile, advanced_run]
assert len(checks) == 14 and all(row['returncode'] == 0 for row in checks)
for row in checks:
    assert sha(Path(row['log'])) == row['log_sha256']
    if 'binary_sha256' in row:
        assert sha(Path(row['command'][-1])) == row['binary_sha256']
units = {row['source']: row for row in checks if 'source' in row}
expected = {
    'test/unit/fps_convolution_borrow.cpp', 'test/unit/fps_convolution.cpp',
    'test/unit/fps_convolution_constant.cpp', 'test/unit/fps_composition_blocked.cpp',
    'test/unit/fps_evaluation.cpp', 'test/unit/fps_tuning_advanced.cpp',
}
assert set(units) == expected
for name, row in units.items():
    assert '-fsanitize=address,undefined' in row['command']
    assert ('-Wno-error=array-bounds' in row['command']) == (name == 'test/unit/fps_evaluation.cpp')
for name, expected_hash in main['source_sha256'].items():
    assert sha(ROOT / name) == expected_hash
proof = dict(initial)
proof.pop('script_sha256')
proof.update(units=sorted(expected), checks=checks, completed=True, sources_unchanged=True,
    replay_harness=dict(path='test/benchmark/fps_refactor_native_sanitize.py', sha256=sha(ROOT / 'test/benchmark/fps_refactor_native_sanitize.py')),
    executed_harness_snapshots={name: sha(RESULT / name) for name in ['native_sanitize_initial_runner.py', 'native_sanitize_evaluation_runner.py']},
    compilation_warning=dict(
        initial_report='native_sanitize_initial.json', initial_report_sha256=sha(RESULT / 'native_sanitize_initial.json'),
        followup_report='native_evaluation_warning.json', followup_report_sha256=sha(RESULT / 'native_evaluation_warning.json'),
        reason='Baseline and final GCC sanitizer builds diagnose the same unreachable Part[1] merge loop at O1; final O2 retains that diagnostic. StandardLayout kBase=0 has positive leaf degrees, so coefficients is called only with count=1 and returns before the loop. Only this evaluation unit adds -Wno-error=array-bounds; the diagnostic and all runtime sanitizers stay enabled.'),
    replaced_basic_attempt=dict(path='native_tuning_basic.json', sha256=sha(RESULT / 'native_tuning_basic.json'),
        reason='The optional basic unit hit a separate GCC sanitizer compile-time array-bounds diagnostic in unchanged sqrt helper code. The authorized advanced alternative was used, without adding a warning exception.'),
    flags_per_unit=evaluation['flags_per_unit'],
    completion_reports={name: sha(RESULT / name) for name in ['native_evaluation_sanitized.json', 'native_tuning_advanced.json']},
    runtime_scope='Linux GCC native AVX2 is available; six units exercise native C++ wrappers with ASan/UBSan and guard-page oracles. Inline asm memory accesses themselves are not instrumented.',
    finalizer_sha256=sha(Path(__file__)))
(RESULT / 'native_sanitize.json').write_text(json.dumps(proof, indent=2) + '\n')
files = ['validation.json', 'native_sanitize.json', 'native_sanitize_initial.json', 'native_evaluation_warning.json',
         'native_evaluation_sanitized.json', 'native_tuning_basic.json', 'native_tuning_advanced.json',
         'native_sanitize_initial_runner.py', 'native_sanitize_evaluation_runner.py']
manifest = dict(completed=True, main_completed=main['completed'], native_completed=proof['completed'],
                baseline_commit=main['baseline_commit'], source_maps_identical=True,
                native_successful_checks=len(checks), native_units=sorted(expected),
                files={name: sha(RESULT / name) for name in files},
                harnesses={name: sha(ROOT / name) for name in ['test/benchmark/fps_refactor_check.py',
                    'test/benchmark/fps_refactor_native_sanitize.py', 'test/benchmark/fps_refactor_native_finalize.py']})
(RESULT / 'native_sanitize_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
print('PASS: main complete; native GCC sanitizer covers six units in 14 successful checks; all sources and logs match')
