import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-output-pages'))
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--trials', type=int, default=21)
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    source = ROOT / 'test/benchmark/fps_output_pages.cpp'
    binary = args.work / 'driver'
    result = dict(compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  kernel=subprocess.check_output(['uname', '-r'], text=True).strip(),
                  hardware=subprocess.check_output(['lscpu'], text=True), flags=flags, cpu=args.cpu,
                  trials=args.trials, seed=20261019, results=[], thp={name: (Path('/sys/kernel/mm/transparent_hugepage') / name).read_text().strip() for name in ['enabled', 'defrag']})
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        subprocess.run(['g++', *flags, '-I', str(ROOT), str(source), '-o', str(binary)], check=True)
        result['sources'] = {str(p.relative_to(ROOT)): sha(p) for p in [source, Path(__file__).resolve(), ROOT / 'components/fps_output.hpp', ROOT / 'components/fps_workspace.hpp', ROOT / 'math/mint.hpp']}
        result['binary_sha256'] = sha(binary)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {args.cpu})
        rng = random.Random(20261019)
        for n in [32768, 500000, 524288, 999999, 1048575]:
            for encoded in [1, 0]:
                samples, checks = {mode: [] for mode in range(4)}, set()
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for mode in order:
                        data = json.loads(subprocess.check_output([str(binary), str(mode), str(n), str(encoded)], text=True))
                        checks.add(data['checksum'])
                        if trial:
                            samples[mode].append(data)
                assert len(checks) == 1
                names = ['current', 'populate', 'huge', 'huge_populate']
                metrics = ['total_ms', 'reserve_ms', 'advice_ms', 'append_ms', 'destroy_ms', 'minor_faults', 'huge_bytes', 'populate_bytes']
                row = dict(n=n, already_montgomery=bool(encoded), variants={names[mode]: dict(medians={key: statistics.median(sample[key] for sample in values) for key in metrics}, samples=values) for mode, values in samples.items()})
                result['results'].append(row)
                print(n, encoded, {name: values['medians']['total_ms'] for name, values in row['variants'].items()}, flush=True)
                args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
