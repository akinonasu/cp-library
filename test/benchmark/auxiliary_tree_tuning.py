import argparse
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--variants', type=Path, default=ROOT / 'test/benchmark/results/virtual_tree_tuning.json')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    variants = json.loads(args.variants.read_text())['variants']
    variants = {name: header.replace('VirtualTree', 'AuxiliaryTree').replace('VIRTUAL_TREE_HPP', 'AUXILIARY_TREE_HPP') for name, header in variants.items()}
    source = (ROOT / 'test/benchmark/auxiliary_tree.cpp').read_text()
    os.sched_setaffinity(0, {2})
    flags = ['-std=c++2b', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero']
    result = {'compiler': subprocess.check_output(['g++', '--version'], text=True),
              'uname': subprocess.check_output(['uname', '-a'], text=True),
              'cpu': next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
              'flags': flags, 'affinity': [2], 'warmup': 1, 'trials': 7, 'schedule_seed': 118249,
              'input_seed': 612409, 'binary_filesystem': 'Linux /tmp (WSL ext4)',
              'metric': 'in-process construction and checksum; same as auxiliary_tree.py',
              'source_sha256': {'test/benchmark/auxiliary_tree.cpp': hashlib.sha256(source.encode()).hexdigest()},
              'variants': variants, 'cases': []}
    with tempfile.TemporaryDirectory(prefix='cp-virtual-tune-') as folder:
        folder = Path(folder)
        binaries = {}
        for name, header in variants.items():
            path = folder / (name + '.hpp')
            path.write_text(header)
            cpp = folder / (name + '.cpp')
            cpp.write_text(source.replace('"tree/auxiliary_tree.hpp"', '"' + str(path) + '"'))
            binary = folder / name
            subprocess.run(['g++', *flags, '-I', str(ROOT), str(cpp), '-o', str(binary)], check=True)
            binaries[name] = binary
        rng = random.Random(118249)
        for shape, k, q in (('path', 32, 20000), ('star', 32, 20000), ('random', 1024, 1000)):
            samples = {name: [] for name in binaries}
            expected = None
            for trial in range(8):
                names = list(binaries)
                rng.shuffle(names)
                for name in names:
                    elapsed, checksum = subprocess.check_output([str(binaries[name]), shape, 'library', '200000', str(k), str(q), 'lca'], text=True).split()
                    if expected is None:
                        expected = checksum
                    assert expected == checksum
                    if trial:
                        samples[name].append(float(elapsed))
            item = {'shape': shape, 'k': k, 'queries': q, 'checksum': expected, 'milliseconds': samples,
                    'median_ms': {name: statistics.median(values) for name, values in samples.items()}}
            result['cases'].append(item)
            print(shape, k, item['median_ms'], flush=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
