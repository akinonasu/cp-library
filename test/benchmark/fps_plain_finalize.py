import collections
import csv
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import shutil
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / 'test/benchmark/results/fps_plain'
sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
load = lambda name: json.loads((RESULT / name).read_text())
core = load('core_checks.json')
official = load('official.json')
native = load('native_sanitize.json')
audit = load('audit.json')
asserts = load('assert_audit.json')
performance = load('performance.json')
sizes = load('sizes.json')
submission = load('submission.json')
distribution = load('distribution.json')
assert all(row['completed'] for row in [official, native, audit, asserts, performance, sizes, submission])
assert official['source_sha256'] == native['source_sha256'] == core['source_sha256']
for name, digest in core['source_sha256'].items():
    assert sha(ROOT / name) == digest, name
for proof in [core, official]:
    assert all(row['returncode'] == 0 and sha(Path(row['log'])) == row['log_sha256'] for row in proof['commands'])
    for compiler, digest in proof['compiler_sha256'].items():
        assert sha(Path(shutil.which(compiler) or compiler).resolve()) == digest
assert official['resumed']['proof_sha256'] == sha(RESULT / 'core_checks.json')
assert official['resumed']['controller_sha256'] == sha(ROOT / 'test/benchmark/fps_plain_resume.py')
assert all(row['tokens_equal'] and row['elf_equal'] for row in audit['pairs']) and len(audit['pairs']) == 7
assert all(row['tokens_equal'] for row in asserts['pairs']) and len(asserts['pairs']) == 7
assert asserts['release_audit_sha256'] == sha(RESULT / 'audit.json')
for report, script in [(audit, 'fps_plain_audit.py'), (asserts, 'fps_plain_assert_audit.py'),
                       (performance, 'fps_256k_refactor.py'), (submission, 'fps_plain_submission.py')]:
    assert report['harness_sha256'] == sha(ROOT / 'test/benchmark' / script)
for name, digest in sizes['source_sha256'].items():
    assert sha(ROOT / name) == digest, name
for name, digest in distribution['inputs'].items():
    assert sha(ROOT / name) == digest, name
for name, info in distribution['outputs'].items():
    assert sha(ROOT / name) == info['sha256'] and (ROOT / name).stat().st_size == info['bytes'], name
originals = {}
for path in sorted([*(ROOT / 'fps').glob('*.inc'), *(ROOT / 'components').glob('*.inc')]):
    header = path.with_suffix('.hpp').relative_to(ROOT).as_posix()
    original = subprocess.check_output(['git', 'show', core['baseline_commit'] + ':' + header], cwd=ROOT)
    assert original == path.read_bytes(), path
    originals[path.relative_to(ROOT).as_posix()] = {'baseline_header': header, 'sha256': sha(path)}
assert len(originals) == 40
compilers = {row['compiler'] for row in core['unit'] if not row['sanitized']}
expected_units = {(name, cxx, False) for name in core['selection']['unit'] for cxx in compilers}
expected_units |= {(name, row['compiler'], True) for name in core['selection']['sanitize']
                   for row in core['unit'] if row['sanitized']}
actual_units = {(row['source'], row['compiler'], row['sanitized']) for row in core['unit']}
assert expected_units == actual_units and len(actual_units) == len(core['unit'])
expected_official = set()
for name in core['selection']['verification']:
    if re.search(r'https://judge\.yosupo\.jp/problem/', (ROOT / name).read_text()):
        expected_official.update((name, cxx) for cxx in compilers)
rows = core['official'] + official['official']
assert {(row['driver'], row['compiler']) for row in rows} == expected_official
assert len(rows) == len(expected_official)
assert official['official_assets_unchanged']
for problem, assets in core['cases'].items():
    if problem in official['cases']:
        assert official['cases'][problem] == assets
    else:
        folder = Path(assets['directory']) if 'directory' in assets else None
        if folder is None:
            repository = Path('/workspace/cp-fps-scratch-round3/library-checker-problems')
            folder = next(repository.glob('*/' + problem + '/info.toml')).parent
        assert sha(folder / 'checker') == assets['checker_sha256']
        for name, proof in assets['inputs'].items():
            assert sha(folder / 'in' / name) == proof['input_sha256']
            assert sha(folder / 'out' / (Path(name).stem + '.out')) == proof['answer_sha256']
for suite, variants in performance['sources'].items():
    for variant, proof in variants.items():
        for name, info in proof['files'].items():
            if name.startswith('@driver/'):
                driver = 'test/benchmark/' + Path(info['path']).name
                original = subprocess.check_output(['git', 'show', core['baseline_commit'] + ':' + driver], cwd=ROOT)
                assert hashlib.sha256(original).hexdigest() == info['sha256'], (suite, name)
                assert sha(ROOT / driver) == info['sha256']
            elif variant == 'after':
                assert sha(Path(info['path'])) == info['sha256'], (suite, name)
            elif not name.startswith('@'):
                original = subprocess.check_output(['git', 'show', core['baseline_commit'] + ':' + name], cwd=ROOT)
                assert hashlib.sha256(original).hexdigest() == info['sha256'], (suite, name)
samples = list(csv.DictReader((RESULT / 'performance.csv').open()))
groups = collections.defaultdict(list)
for row in samples:
    groups[row['suite'], row['operation'], int(row['n']), row['variant']].append(row)
for result in performance['results']:
    answers = set()
    for variant in ['before', 'after']:
        group = groups[result['suite'], result['operation'], result['n'], variant]
        assert len(group) == performance['trials']
        assert statistics.median(float(row['ms']) for row in group) == result['median_ms'][variant]
        answers.update((row['result_size'], row['second_size'], row['third_size'], row['checksum']) for row in group)
    assert len(answers) == 1
assert len(samples) == len(performance['results']) * 2 * performance['trials'] == 1314
assert not (ROOT / 'tools/bundle.py').exists()
assert submission['source_sha256'] == sha(ROOT / 'submissions/kth_term_of_linearly_recurrent_sequence.cpp')
reports = ['core_checks.json', 'official.json', 'native_sanitize.json', 'audit.json', 'assert_audit.json',
           'performance.json', 'performance.csv', 'sizes.json', 'submission.json', 'distribution.json']
controls = ['tools/update_fps_headers.py', 'tools/compact_fps.py', '.verify-helper/documentation.py',
            '.verify-helper/bundle_test.py', '.verify-helper/ci.py', '.verify-helper/check-docs.py',
            'test/benchmark/fps_plain_finalize.py', 'test/benchmark/source_size_budget.py',
            'test/benchmark/fps_plain_resume.py', 'test/benchmark/fps_plain_assert_audit.py',
            'test/benchmark/fps_plain_audit.py', 'test/benchmark/fps_plain_submission.py']
certificate = {'completed': True, 'completed_at': datetime.now(timezone.utc).isoformat(),
    'baseline_commit': core['baseline_commit'], 'source_sha256': core['source_sha256'],
    'compiler_sha256': core['compiler_sha256'], 'reports': {name: sha(RESULT / name) for name in reports},
    'controls': {name: sha(ROOT / name) for name in controls}, 'original_sources': originals,
    'standalone': len(core['standalone']), 'unit': sum(not row['sanitized'] for row in core['unit']),
    'clang_sanitizer': sum(row['sanitized'] for row in core['unit']), 'gcc_native_sanitizer': len(native['units']),
    'integration': len(core['integration']), 'maximum_precision': len(core['precision']),
    'official_driver_compiler_pairs': len(rows), 'official_drivers': len({row['driver'] for row in rows}),
    'official_cases': sum(row['cases_passed'] for row in rows),
    'official_problems': sorted({row['problem'] for row in rows}),
    'external_drivers': official['skipped_external'], 'release_equivalence_pairs': len(audit['pairs']),
    'assertion_equivalence_pairs': len(asserts['pairs']), 'size_checks': len(sizes['sizes']),
    'header_count': sizes['header_count'], 'judge_count': sizes['judge_count'],
    'maximum_copy_bytes': sizes['maximum_copy_bytes'], 'performance_conditions': len(performance['results']),
    'performance_samples': len(samples), 'bundle_tool_removed': True,
    'scope': 'Core stages and two completed official drivers reused with identical source/compiler hashes; remaining official drivers completed after preparing two missing fixtures. All selected Library Checker driver/compiler pairs are covered.'}
(RESULT / 'validation.json').write_text(json.dumps(certificate, indent=2) + '\n')
print('Verified', certificate['official_cases'], 'official cases;', certificate['unit'], 'units;', len(samples), 'benchmark samples')
