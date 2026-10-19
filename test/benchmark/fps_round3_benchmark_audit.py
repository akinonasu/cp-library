import ast
import hashlib
import io
import json
import math
from pathlib import Path
import re
import statistics
import subprocess
import tarfile

ROOT = Path(__file__).resolve().parents[2]
RESULTS = ROOT / 'test/benchmark/results'
OUT = RESULTS / 'fps_round3_validation'
BASELINE = 'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0'
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
HARNESS = 'c48f7be3294b9cc1de1b43e6ef54a820a1097559068f1fae5b17b1d6b6e3ac47'
RAW = '360af00d2182ce18c0c963a537220534d877c823b43a3fbf3d32923c9a5a1237'
OLD = 'bbe4528ee226bcf0ea90d143f0b5832ef6f69b2162b448ec845d249349bc7be3'
NEW = 'bda7e2f7b2189023cc5da6b695eddc5bcde4ff81eb8407d933c0cf18fb13e258'
BOSTAN = 'components/fps_bostan.hpp'
PROBLEMS = ['convolution_mod', 'inv_of_formal_power_series', 'log_of_formal_power_series',
            'exp_of_formal_power_series', 'pow_of_formal_power_series', 'sqrt_of_formal_power_series',
            'division_of_polynomials', 'kth_term_of_linearly_recurrent_sequence',
            'composition_of_formal_power_series', 'composition_of_formal_power_series_large',
            'compositional_inverse_of_formal_power_series', 'compositional_inverse_of_formal_power_series_large',
            'multipoint_evaluation', 'polynomial_interpolation', 'polynomial_taylor_shift', 'inv_of_polynomials']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def need(ok, message):
    if not ok:
        raise RuntimeError(message)


def closure(source, tree):
    pending, visited = [source.resolve()], set()
    while pending:
        path = pending.pop()
        if path in visited:
            continue
        visited.add(path)
        for text in re.findall(r'^\s*#\s*include\b([^\n]*)', path.read_text().replace('\\\n', ''), re.M):
            match = re.match(r'\s*(?:"([^"]+)"|<([^>]+)>)', text)
            need(match is not None, 'Unresolved include syntax in ' + str(path))
            quoted, angled = match.groups()
            candidates = [path.parent / quoted, tree / quoted] if quoted else [tree / angled]
            found = next((p.resolve() for p in candidates if p.is_file()), None)
            need(found is not None or quoted is None, 'Missing quoted include in ' + str(path))
            if found is not None:
                need(found.is_relative_to(tree), 'Local include escapes snapshot: ' + str(found))
                pending.append(found)
    return visited


def paired(a, b):
    diff = [x - y for x, y in zip(a, b)]
    return dict(median_ms=statistics.median(diff), wins=sum(x > 0 for x in diff),
                ties=sum(x == 0 for x in diff), losses=sum(x < 0 for x in diff))


def main():
    raw_path = RESULTS / 'fps_round3_all_speed.json'
    tracked = [raw_path, OUT / 'bostan_pragma_normalization.json', OUT / 'judge_official_before.json',
               RESULTS / 'fps_round3_reference_heads.json', RESULTS / 'fps_speed_reference.json',
               ROOT / 'test/benchmark/fps_all_speed.py', ROOT / 'test/benchmark/fps_round3_references.py']
    initial = {str(p): sha(p) for p in tracked}
    read = lambda p: json.loads(p.read_text())
    raw, norm, official, heads, references = map(read, tracked[:5])
    need(initial[str(raw_path)] == RAW == norm['parent_raw']['sha256'], 'Primary raw hash mismatch')
    need(raw['baseline'] == BASELINE and raw['flags'] == FLAGS, 'Baseline/flags mismatch')
    need(raw['harness_sha256'] == HARNESS == sha(tracked[5]), 'Formal harness mismatch')
    need(raw['trials'] == 21 and raw['cpu'] == 2, 'Trial count or CPU mismatch')
    need([row['problem'] for row in raw['judge']] == PROBLEMS, '16-workload scope mismatch')
    need(heads['harness_sha256'] == sha(tracked[6]), 'Fresh-head harness mismatch')
    conditions = read(OUT / 'conditions.json')
    need(raw['compiler'] == conditions['versions']['gcc14'].splitlines()[0] == norm['compiler'], 'Compiler mismatch')
    need(sha(Path(conditions['compilers']['gcc14']['path']).resolve()) == conditions['compilers']['gcc14']['sha256'], 'Compiler executable changed')
    trees = {v: Path(raw['sources']['convolution_mod_' + v]['command'][-3]).parents[1]
             for v in ['before', 'current']}
    baseline_paths = set(raw['sources']['before'])
    baseline_paths.update(str(Path(raw['sources'][p + '_before']['command'][-3]).relative_to(trees['before'])) for p in PROBLEMS)
    baseline_paths.add('test/benchmark/results/fps_speed_reference.json')
    archive = subprocess.check_output(['git', 'archive', BASELINE, '--', *sorted(baseline_paths)], cwd=ROOT)
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        baseline_files = {name: tar.extractfile(name).read() for name in baseline_paths}
    need(baseline_files['test/benchmark/results/fps_speed_reference.json'] == tracked[4].read_bytes(), 'Reference metadata changed from baseline')
    header_audit = {}
    extra_current = set()
    for variant, tree in trees.items():
        recorded = raw['sources'][variant]
        actual = {str(p.relative_to(tree)): sha(p) for p in tree.rglob('*.hpp')}
        need(set(recorded) <= actual.keys(), 'Snapshot header missing: ' + variant)
        delta = {name: [digest, actual[name]] for name, digest in recorded.items() if actual[name] != digest}
        need(delta == ({} if variant == 'before' else {BOSTAN: [OLD, NEW]}), 'Unexpected header delta: ' + variant)
        if variant == 'before':
            need(set(recorded) == actual.keys(), 'Unrecorded baseline header')
            need(all(hashlib.sha256(baseline_files[name]).hexdigest() == digest for name, digest in recorded.items()), 'Baseline header differs from Git')
        else:
            extra_current = set(actual) - set(recorded)
            need(all((ROOT / name).is_file() and sha(ROOT / name) == digest for name, digest in actual.items()), 'Current snapshot differs from live repository')
            need(norm['normalized_sources_current'] == {name: actual[name] for name in recorded}, 'Canonical header manifest mismatch')
        header_audit[variant] = dict(recorded=len(recorded), actual=len(actual), delta=delta,
                                     auxiliary_headers={name: actual[name] for name in sorted(set(actual) - set(recorded))})
    need(norm['parent_raw']['historical_sources_current'] == raw['sources']['current'], 'Historical header binding mismatch')
    need(norm['parent_raw_unchanged'] and norm['byte_identical'] and norm['cmp_returncode'] == 0 and norm['build']['returncode'] == 0, 'Normalization proof failed')
    need(norm['flags'] == FLAGS and norm['parent_raw']['completed_workloads'] == 16, 'Normalization scope mismatch')
    for key, expected in [('historical_header', OLD), ('canonical_header', NEW)]:
        row = norm[key]
        need(row['sha256'] == row['archived_sha256'] == expected == sha(Path(row['archived_source'])), 'Pragma archived source mismatch')
    old = Path(norm['historical_header']['archived_source']).read_bytes()
    new = Path(norm['canonical_header']['archived_source']).read_bytes()
    need(old.count(b'#pragma GCC target("avx2")') == 1 and old.replace(b'#pragma GCC target("avx2")', b'_Pragma("GCC target(\\"avx2\\")")', 1) == new, 'Pragma change is not exact')
    lr = raw['sources']['kth_term_of_linearly_recurrent_sequence_current']
    need(norm['parent_raw']['historical_binary_record'] == lr and norm['command'] == lr['command'], 'LR command binding mismatch')
    for key in ['binary_before', 'binary_after']:
        row = norm[key]
        need(sha(Path(row['path'])) == row['sha256'] == lr['binary_sha256'], 'LR binary proof mismatch')
        need(Path(row['path']).stat().st_size == row['bytes'], 'LR binary size mismatch')
    official_root = Path('/tmp/library-checker-problems').resolve()
    need(raw['official_commit'] == official['commit'] == subprocess.check_output(['git', '-C', str(official_root), 'rev-parse', 'HEAD'], text=True).strip(), 'Official commit mismatch')
    folders = {p.parent.name: p.parent for p in official_root.glob('*/*/info.toml')}
    rows, checked_files = [], {}
    for row in raw['judge']:
        problem = row['problem']
        need(set(row['variants']) == {'before', 'current', 'reference'}, 'Variant scope mismatch: ' + problem)
        reference = next(r for r in references['problems'][problem]['references'] if r['lang'] == 'cpp')
        fresh = heads['problems'][problem]
        need(row['reference'] == reference and fresh['fastest_unchanged'], 'Reference mismatch: ' + problem)
        need(fresh['top_cpp'][0]['id'] == reference['id'] == next(r['id'] for r in fresh['listing']['submissions'] if r['lang'] == 'cpp'), 'Fresh fastest mismatch: ' + problem)
        folder = folders[problem]
        files = {'input': folder / 'in' / row['input'], 'answer': folder / 'out' / (Path(row['input']).stem + '.out'), 'checker': folder / 'checker'}
        official_hashes = {}
        for kind, path in files.items():
            relative = path.relative_to(official_root).as_posix()
            digest = sha(path)
            need(official['files'].get(relative) == digest, 'Official manifest mismatch: ' + relative)
            need(kind == 'checker' or digest == row[kind + '_sha256'], 'Raw official hash mismatch: ' + relative)
            official_hashes[kind] = dict(path=str(path), sha256=digest)
        variants = {}
        for variant, data in row['variants'].items():
            samples = data['samples_ms']
            need(len(samples) == 21 and all(isinstance(x, (int, float)) and math.isfinite(x) and x > 0 for x in samples), 'Invalid samples: ' + problem + '/' + variant)
            need(statistics.median(samples) == data['median_ms'], 'Median mismatch: ' + problem + '/' + variant)
            record = raw['sources'][problem + '_' + variant]
            command = record['command']
            source, binary, tree = Path(command[-3]), Path(command[-1]), Path(command[-4])
            need(command[:1 + len(FLAGS)] == ['g++', *FLAGS] and command[-5] == '-I' and command[-2] == '-o' and tree == trees['current' if variant == 'reference' else variant], 'Command/flags mismatch: ' + problem + '/' + variant)
            output = Path(str(binary) + '.out')
            need(sha(source) == record['source_sha256'] and sha(binary) == record['binary_sha256'] and sha(output) == data['output_sha256'], 'Source/binary/output hash mismatch: ' + problem + '/' + variant)
            if variant == 'reference':
                need(source.name == str(reference['id']) + '.cpp' and record['source_sha256'] == reference['source_sha256'], 'Reference source mismatch: ' + problem)
                need(not re.search(r'^\s*#\s*include\s*"', source.read_text(), re.M), 'Reference is not standalone: ' + problem)
            else:
                name = source.relative_to(tree).as_posix()
                if variant == 'before':
                    need(source.read_bytes() == baseline_files[name], 'Baseline driver differs from Git: ' + name)
                else:
                    need(source.read_bytes() == (ROOT / name).read_bytes(), 'Current driver differs from live source: ' + name)
                    dependency_headers = {p.relative_to(tree).as_posix() for p in closure(source, tree) if p.suffix == '.hpp'}
                    need(not (dependency_headers & extra_current), 'Auxiliary header is a native-driver dependency')
            variants[variant] = dict(source=str(source), source_sha256=record['source_sha256'], binary=str(binary), binary_sha256=record['binary_sha256'], output=str(output), output_sha256=data['output_sha256'], samples=21, median_ms=data['median_ms'])
            checked_files.update({str(source): record['source_sha256'], str(binary): record['binary_sha256'], str(output): data['output_sha256']})
        rows.append(dict(problem=problem, reference_id=reference['id'], fresh_fastest_matches=True, official=official_hashes, variants=variants,
                         paired_before_minus_current=paired(row['variants']['before']['samples_ms'], row['variants']['current']['samples_ms']),
                         paired_reference_minus_current=paired(row['variants']['reference']['samples_ms'], row['variants']['current']['samples_ms'])))
    need({str(p): sha(p) for p in tracked} == initial and sha(raw_path) == RAW, 'Raw/provenance files changed during audit')
    need(all(sha(Path(path)) == digest for path, digest in checked_files.items()), 'Measured artifact changed during audit')
    result = dict(status='passed', baseline=BASELINE, flags=FLAGS, cpu=2, workloads=16, variants_per_workload=3,
                  samples_per_variant=21, total_timed_samples=1008, source_binary_output_records=48,
                  primary_raw=dict(path=str(raw_path), sha256_before=RAW, sha256_after=sha(raw_path), unchanged=True),
                  header_audit=header_audit, auxiliary_headers_excluded_by_static_native_driver_closure=True,
                  normalization=dict(path=str(tracked[1]), sha256=initial[str(tracked[1])], exact_one_header=True, lr_binary_byte_identical=True),
                  checker_hash_provenance='judge_official_before.json, also matched against existing checker files; no checker rerun',
                  official_commit=raw['official_commit'], fresh_heads_retrieved_at=heads['retrieved_at'],
                  provenance_hashes=initial, audited_harness_sha256=HARNESS, audit_harness_sha256=sha(Path(__file__)), rows=rows)
    (OUT / 'benchmark_audit.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(dict(status='passed', workloads=16, records=48, samples=1008, raw_unchanged=True)))


if __name__ == '__main__':
    ast.parse(Path(__file__).read_text())
    main()
