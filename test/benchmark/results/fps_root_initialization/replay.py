import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

HERE = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    if args.work.exists():
        raise RuntimeError('Use a new --work directory')
    args.work.mkdir(parents=True)
    repo, work = args.repo.resolve(), args.work.resolve()
    subprocess.run([sys.executable, str(HERE / 'restore.py'), '--repo', str(repo), '--work', str(work / 'source-check')], check=True)
    raw = json.loads((HERE / 'raw.json').read_text())
    text = (HERE / 'original_harness.py').read_text()
    substitutions = {
        "R=Path('/workspace/cp-library');W=Path('/tmp/fps-zero-roots');base='4765ceb';":
            f"R=Path({str(repo)!r});W=Path({str(work / 'measurement')!r});base={raw['baseline']!r};",
        "R/'test/benchmark/fps_tuning_basic.cpp'":
            f"Path({str(work / 'source-check/baseline/test/benchmark/fps_tuning_basic.cpp')!r})",
        "Path('/tmp/library-checker-problems/polynomial')":
            f"Path({str(args.official_root.resolve() / 'polynomial')!r})",
    }
    for old, new in substitutions.items():
        if text.count(old) != 1:
            raise RuntimeError(('Expected one substitution', old))
        text = text.replace(old, new)
    compile(text, 'replay_harness.py', 'exec')
    (work / 'measurement').mkdir()
    script = work / 'replay_harness.py'
    script.write_text(text)
    metadata = dict(baseline=raw['baseline'], original_harness_sha256=raw['harness_sha256'],
                    replay_harness_sha256=hashlib.sha256(script.read_bytes()).hexdigest(),
                    changes='paths and baseline full SHA only; flags, candidate patch, seed, trials and measurements unchanged',
                    prepared=True, measured=False)
    if not args.prepare_only:
        subprocess.run([sys.executable, str(script)], check=True)
        metadata['measured'] = True
    (work / 'replay.json').write_text(json.dumps(metadata, indent=2) + '\n')
    print(json.dumps(metadata, indent=2))


if __name__ == '__main__':
    main()
