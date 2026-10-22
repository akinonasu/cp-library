import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description='Finish official checks after fixture preparation')
    parser.add_argument('--proof', type=Path, required=True)
    parser.add_argument('--problems', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    proof = json.loads(args.proof.read_text())
    assert not proof['completed']
    assert all(row.get('returncode') == 0 for row in proof['commands'])
    for name, digest in proof['source_sha256'].items():
        assert sha(ROOT / name) == digest, name
    assert all(sha(Path(row['log'])) == row['log_sha256'] for row in proof['commands'])
    script = ROOT / 'test/benchmark/fps_refactor_check.py'
    assert sha(script) == proof['script_sha256']
    compilers = list(dict.fromkeys(row['compiler'] for row in proof['official']))
    assert len(compilers) == 2
    passed = {(row['driver'], row['compiler']) for row in proof['official']}
    reused = {name for name in proof['selection']['verification'] if all((name, cxx) in passed for cxx in compilers)}
    spec = importlib.util.spec_from_file_location('fps_checks', script)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    inventory = module.inventory

    def remaining(baseline):
        candidates, selected, changed, sources, precision = inventory(baseline)
        assert sources == proof['source_sha256']
        assert selected == proof['selection']
        selected['verification'] = [name for name in selected['verification'] if name not in reused]
        return candidates, selected, changed, sources, precision

    module.inventory = remaining
    run_args = argparse.Namespace(baseline=proof['baseline_commit'], problems=args.problems.resolve(), acl=None,
        cxx=compilers, sanitize_cxx=compilers[-1], stages=['official'], timeout=300, jobs=2, prepare_only=False,
        work=args.work.resolve(), output=args.output.resolve())
    module.run(run_args)
    result = json.loads(args.output.read_text())
    result['resumed'] = {'proof_sha256': sha(args.proof), 'controller_sha256': sha(Path(__file__)),
                         'reused_drivers': sorted(reused), 'reused_official_rows': proof['official']}
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
