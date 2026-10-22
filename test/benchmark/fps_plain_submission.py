import argparse
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--problems', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--cxx', nargs='+', required=True)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=False)
    source = ROOT / 'submissions/kth_term_of_linearly_recurrent_sequence.cpp'
    folder = next(args.problems.glob('*/kth_term_of_linearly_recurrent_sequence/info.toml')).parent
    inputs = sorted((folder / 'in').glob('*.in'))
    selected = sorted(set([inputs[0], max(inputs, key=lambda path: path.stat().st_size), inputs[-1]]))
    sha = lambda path: hashlib.sha256(path.read_bytes()).hexdigest()
    report = {'completed': False, 'source_sha256': sha(source), 'bytes': source.stat().st_size,
              'harness_sha256': sha(Path(__file__)), 'checker_sha256': sha(folder / 'checker'), 'checks': []}
    assert report['bytes'] < 262144
    for index, compiler in enumerate(args.cxx):
        binary = args.work / str(index)
        command = [compiler, '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-DEVAL', '-DONLINE_JUDGE',
                   str(source), '-o', str(binary)]
        log = binary.with_suffix('.log')
        with log.open('wb') as err:
            subprocess.run(command, stdout=err, stderr=err, check=True)
        row = {'command': command, 'binary_sha256': sha(binary), 'log_sha256': sha(log), 'cases': []}
        for case in selected:
            actual = args.work / 'actual.out'
            with case.open('rb') as inp, actual.open('wb') as out:
                subprocess.run([str(binary)], stdin=inp, stdout=out, check=True)
            expected = folder / 'out' / (case.stem + '.out')
            checked = subprocess.run([str(folder / 'checker'), str(case), str(actual), str(expected)], capture_output=True)
            assert checked.returncode == 0, checked.stderr
            row['cases'].append({'name': case.name, 'input_sha256': sha(case), 'answer_sha256': sha(expected),
                                 'actual_sha256': sha(actual)})
        report['checks'].append(row)
    assert sha(source) == report['source_sha256']
    report['completed'] = True
    args.output.write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
