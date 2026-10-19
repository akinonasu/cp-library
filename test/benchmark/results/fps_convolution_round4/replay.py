import argparse, hashlib, importlib.util, json, lzma, shutil, subprocess, sys, tarfile
from pathlib import Path
p = argparse.ArgumentParser()
p.add_argument('--work', type=Path, required=True)
p.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
p.add_argument('--build-and-measure', action='store_true')
a = p.parse_args()
here = Path(__file__).resolve().parent
source = a.work.resolve() / 'source'
source.mkdir(parents=True)
(source / 'before').mkdir()
with tarfile.open(here / 'baseline.tar.xz') as archive:
    archive.extractall(source / 'before', filter='data')
shutil.copytree(source / 'before', source / 'current')
with (here / 'current.patch').open('rb') as patch:
    subprocess.run(['patch', '-s', '-p1'], cwd=source / 'current', stdin=patch, check=True)
(source / 'reference.cpp').write_bytes(lzma.decompress((here / 'reference.cpp.xz').read_bytes()))
shutil.copyfile(here / 'sources.json', source / 'sources.json')
metadata = json.loads((source / 'sources.json').read_text())
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
for variant in ['before', 'current']:
    actual = {str(p.relative_to(source / variant)): sha(p) for p in sorted((source / variant).rglob('*')) if p.is_file()}
    assert actual == metadata[variant]
assert sha(source / 'reference.cpp') == metadata['reference_sha256']
print('source reconstruction verified', len(metadata['before']), len(metadata['current']), flush=True)
if a.build_and_measure:
    recorded = json.loads((here / 'primary.json').read_text())
    assert sha(here / 'harness.py') == recorded['harness_sha256']
    official = a.official.resolve()
    head = subprocess.check_output(['git', '-C', str(official), 'rev-parse', 'HEAD'], text=True).strip()
    assert head == recorded['official_head'], 'official problem revision differs'
    assert sha(official / 'checker') == recorded['checker']['sha256'], 'checker differs'
    compiler = recorded['compiler']['command']
    assert sha(Path(shutil.which(compiler))) == recorded['compiler']['sha256'], 'compiler differs'
    assert subprocess.check_output([compiler, '--version'], text=True).strip() == recorded['compiler']['version']
    for case in recorded['cases']:
        if case['kind'] == 'official':
            assert sha(official / 'in' / (case['name'] + '.in')) == case['input_sha256']
            assert sha(official / 'out' / (case['name'] + '.out')) == case['answer_sha256']
    spec = importlib.util.spec_from_file_location('recorded_harness', here / 'harness.py')
    harness = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(harness)
    assert harness.FLAGS == recorded['flags']
    original_prepare = harness.prepare_cases
    expected = {c['name']: c for c in recorded['cases']}
    def verified_prepare(args, builds):
        for variant, build in builds.items():
            assert sorted(build['dependencies_before'].values()) == sorted(
                recorded['builds'][variant]['dependencies_before'].values()), (variant, 'dependencies')
        cases = original_prepare(args, builds)
        assert {c['name'] for c in cases} == set(expected)
        for case in cases:
            for key in ['input_sha256', 'answer_sha256', 'dimensions', 'kind']:
                assert case[key] == expected[case['name']][key], (case['name'], key)
        return cases
    harness.prepare_cases = verified_prepare
    harness.REPLAY = Path(__file__).read_text()
    sys.argv = [str(here / 'harness.py'), '--prepared', str(source), '--work', str(a.work),
                '--official', str(official), '--custom', str(a.work / 'custom'),
                '--output', str(a.work / 'results'), '--cxx', compiler,
                '--trials', str(recorded['trials']), '--seed', str(recorded['seed'])]
    harness.main()
