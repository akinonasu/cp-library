import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

import yaml


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--skip-unit', action='store_true')
    parser.add_argument('--policy-only', action='store_true')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = root / 'test/benchmark/results/kinetic_tournament'
    selection = output / ('policy_selection.json' if args.policy_only else 'selection.json')
    previous = json.loads((output / 'validation.json').read_text()) if args.policy_only else None
    if args.policy_only:
        for name in ['data_structure/advanced/generic_segtree_beats.hpp', 'test/unit/generic_segtree_beats.cpp',
                     'test/generic_segtree_beats.test.cpp', 'test/fixtures/beats.hpp']:
            assert hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() == previous['sources'][name]

    def run(command, **kwargs):
        return subprocess.run(command, cwd=root, check=True, **kwargs)

    if not args.skip_unit:
        with (output / 'check.log').open('w') as stream:
            run([sys.executable, 'test/verify.py', '--unit-only', '--cxx', 'g++', 'clang++',
                 '--selection', str(selection)], stdout=stream, stderr=subprocess.STDOUT)
    assert 'Verification passed' in (output / 'check.log').read_text()
    with (output / 'sanitize.log').open('w') as stream:
        run([sys.executable, 'test/verify.py', '--unit-only', '--sanitize', '--cxx', 'clang++',
             '--selection', str(selection)], stdout=stream, stderr=subprocess.STDOUT)
    print('unit and sanitizer passed', flush=True)
    url = 'https://judge.yosupo.jp/problem/range_add_range_min'
    cases = root / '.verify-helper/cache' / hashlib.md5(url.encode()).hexdigest() / 'test'
    if not list(cases.glob('*.in')):
        with (output / 'download.log').open('w') as stream:
            run(['oj', 'download', '--system', '--directory', str(cases), url], stdout=stream, stderr=subprocess.STDOUT)
    inputs = sorted(cases.glob('*.in'))
    assert inputs
    generic_url = 'https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum'
    generic_cases = root / '.verify-helper/cache' / hashlib.md5(generic_url.encode()).hexdigest() / 'test'
    generic_inputs = sorted(generic_cases.glob('*.in'))
    assert generic_inputs, 'Download range_chmin_chmax_add_range_sum first'
    if args.policy_only:
        assert previous['generic_cases'] == {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in generic_inputs}
        for compiler in ['g++', 'clang++']:
            assert previous['compilers'][compiler] == subprocess.check_output([compiler, '--version'], text=True)
    source = 'test/kinetic_tournament.test.cpp'
    header = 'data_structure/advanced/kinetic_tournament.hpp'
    text = (root / 'docs/kinetic_tournament.md').read_text()
    assert yaml.safe_load(text.split('---', 2)[1])['documentation_of'] == '//' + header
    assert '`' + header + '`' in (root / 'README.md').read_text()
    assert '{{' not in text
    examples = re.findall(r'```cpp\n(.*?)\n```', text, re.S)
    samples = [([5, 3, 6, 1, 4, 7], 23), ([22, 7, 20, 26, 1, 11, 7, 1, 15, 10], 104),
               ([140039457, 164832983, 239276621, 245532751, 274195604, 320064892, 499774361, 419370025,
                 249168130, 521915711, 744280520, 561032101, 857077284, 543856068, 910500852], 4618736183),
               ([596982638, 713892371, 987307025, 722757772, 759622047, 853214705, 628902494, 623807668,
                 755031424, 486834509, 272961036, 423817262, 434140395, 284114341, 88860862, 222135106,
                 69837030, 210989591, 225447688, 86416231], 9047041486)]
    checks = []
    with tempfile.TemporaryDirectory(prefix='kinetic-range-check-') as temporary:
        executable = str(Path(temporary) / 'check')
        for compiler in ['g++', 'clang++']:
            flags = [compiler, '-std=c++2b', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(root)]
            run([*flags, source, '-o', executable])
            for path in inputs:
                with path.open('rb') as stream:
                    actual = subprocess.check_output([executable], stdin=stream, timeout=60)
                assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
            for target in [source, header, 'test/benchmark/kinetic_tournament_jsc.cpp']:
                bundle = subprocess.check_output(['oj-bundle', target], cwd=root, text=True, stderr=subprocess.DEVNULL)
                assert '#include "' not in bundle
                run([*flags, '-fsyntax-only', '-x', 'c++', '-'], input=bundle, text=True)
            for example in examples:
                run([*flags, '-x', 'c++', '-', '-o', executable], input=example.replace('"lib/', '"'), text=True)
                run([executable])
            run([*flags, 'test/benchmark/kinetic_tournament_jsc.cpp', '-o', executable])
            for values, answer in samples:
                data = str(len(values)) + '\n' + ' '.join(map(str, values)) + '\n'
                assert int(subprocess.check_output([executable], input=data, text=True)) == answer
            checks.append({'compiler': compiler, 'official_cases': len(inputs), 'jsc_samples': len(samples),
                           'bundles': 'passed', 'examples': len(examples)})
            print(compiler, len(inputs), 'official cases, 4 JSC samples, bundles and example passed', flush=True)
            if not args.policy_only:
                run([*flags, 'test/generic_segtree_beats.test.cpp', '-o', executable])
                for path in generic_inputs:
                    with path.open('rb') as stream:
                        actual = subprocess.check_output([executable], stdin=stream, timeout=60)
                    assert actual.split() == path.with_suffix('.out').read_bytes().split(), (compiler, path)
            checks[-1]['generic_beats_official_cases'] = len(generic_inputs)
            checks[-1]['generic_beats_reused'] = args.policy_only
            print(compiler, 'generic beats', len(generic_inputs), 'official cases ' + ('unchanged' if args.policy_only else 'passed'), flush=True)
    names = [source, header, 'components/kinetic_tournament_monoid.hpp', 'data_structure/advanced/generic_segtree_beats.hpp',
             'docs/kinetic_tournament.md', 'test/unit/kinetic_tournament.cpp', 'test/unit/standalone.cpp',
             'test/benchmark/kinetic_tournament_jsc.cpp', 'components/fast_io.hpp', 'docs/generic_segtree_beats.md',
             'test/unit/generic_segtree_beats.cpp', 'test/generic_segtree_beats.test.cpp', 'test/fixtures/beats.hpp']
    result = {'result': 'passed', 'checks': checks, 'jsc_samples': samples,
              'cases': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs},
              'generic_cases': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in generic_inputs},
              'sources': {name: hashlib.sha256((root / name).read_bytes().replace(b'\r\n', b'\n')).hexdigest() for name in names},
              'compilers': {c: subprocess.check_output([c, '--version'], text=True) for c in ['g++', 'clang++']}}
    (output / 'validation.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
