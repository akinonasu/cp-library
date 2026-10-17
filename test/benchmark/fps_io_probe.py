import argparse
import array
from contextlib import contextmanager
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import struct
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference', type=Path, default=Path('.verify-helper/cache/fps-tuning-reference/convolution_mod/409465.cpp'))
    parser.add_argument('--official', type=Path, default=Path.home() / '.cache/online-judge-tools/library-checker-problems/convolution/convolution_mod')
    parser.add_argument('--trials', type=int, default=7)
    args = parser.parse_args()
    os.sched_setaffinity(0, {2})
    args.output.mkdir(parents=True, exist_ok=True)
    lock = open('/tmp/cp-fps-tuning.lock', 'w')

    @contextmanager
    def exclusive():
        fcntl.flock(lock, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)

    source = args.reference.read_text()
    prefix = source[:source.index('// lib/ntt/ntt.hpp')]
    fields = source[source.index('// problems/convolution/convolution_mod/fields.hpp'):source.index('}  // namespace fields') + len('}  // namespace fields')]
    fixed = source[source.index('// Input fast paths for inputs'):source.index('char* text(Product& product)')]
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-march=native', '-ftrivial-auto-var-init=zero']
    result = dict(date=datetime.now(timezone.utc).isoformat(), reference='https://judge.yosupo.jp/submission/409465',
                  reference_sha256=sha(args.reference), flags=flags, compiler=subprocess.check_output(['g++', '--version'], text=True),
                  cpu=next(line for line in Path('/proc/cpuinfo').read_text().splitlines() if line.startswith('model name')),
                  bulk_macros={'native':0, 'transposed':1},
                  affinity=[2], trials=args.trials, warmup=1, seed=409465,
                  adaptation='extract original IO prefix, fields namespace and fixed_width/read_values; omit every convolution kernel and entry point',
                  method='in-process wall time; read includes Reader construction/destruction and destination resize allocation, excluding destination destruction; write excludes binary fixture loading, Mint setup and destination/Mint destruction, includes output object construction and flushing; checksum outside timing; regular files on Linux /tmp',
                  source_sha256={p:sha(ROOT / p) for p in ('test/benchmark/fps_io_probe.cpp', 'test/benchmark/fps_io_probe.py', 'components/fast_io.hpp', 'math/mint.hpp')},
                  workloads=[])
    rng = random.Random(409465)
    with tempfile.TemporaryDirectory(prefix='fps-io-probe-') as temporary:
        work = Path(temporary)
        header = work / 'reference.hpp'
        header.write_text(prefix + '\n' + fields + '\n' + fixed)
        result['adapted_sha256'] = sha(header)
        binaries = {}
        for name, extra in [('native', ['-DIO_BULK32_TRANSPOSE=0']), ('transposed', ['-DIO_BULK32_TRANSPOSE=1'])]:
            with exclusive():
                binary = work / name
                subprocess.run(['g++', *flags, *extra, f'-DFPS_IO_PROBE_REFERENCE="{header}"', '-I', str(ROOT), str(ROOT / 'test/benchmark/fps_io_probe.cpp'), '-o', str(binary)], check=True)
                binaries[name] = binary
        for stem in ('all_same_00', 'all_same_01', 'max_random_00'):
            with exclusive():
                inp = work / (stem + '.in')
                inp.write_bytes((args.official / 'in' / (stem + '.in')).read_bytes())
                histogram = {}
                for token in inp.read_bytes().split()[2:]:
                    key = str(len(token))
                    histogram[key] = histogram.get(key, 0) + 1
                answer = args.official / 'out' / (stem + '.out')
                values = array.array('I', map(int, answer.read_bytes().split()))
                fixture = work / (stem + '.bin')
                fixture.write_bytes(struct.pack('<Q', len(values)) + values.tobytes())
                wanted = values.tobytes()
            for operation, names in [('read', ['own_raw', 'own_mint', 'ref_scalar', 'ref_bulk', 'ref_fixed', 'ref_transposed']),
                                     ('write', ['own_raw', 'own_mint', 'ref_scalar', 'ref_fields'])]:
                samples = {name: [] for name in names}
                expected = None
                hashes = {}
                with exclusive():
                    for trial in range(args.trials + 1):
                        order = names.copy()
                        rng.shuffle(order)
                        for name in order:
                            binary = binaries['transposed' if name == 'ref_transposed' else 'native']
                            variant = 'ref_bulk' if name == 'ref_transposed' else name
                            output, log = work / 'output.txt', work / 'log.txt'
                            with (inp if operation == 'read' else fixture).open('rb') as incoming, output.open('wb') as outgoing, log.open('w') as error:
                                subprocess.run([binary, operation, variant], stdin=incoming, stdout=outgoing, stderr=error, check=True)
                            elapsed, checksum, count = log.read_text().split()
                            if expected is None:
                                expected = checksum, count
                            assert (checksum, count) == expected, (stem, operation, name)
                            if operation == 'write':
                                digest = sha(output)
                                if hashes.get(name) != digest:
                                    assert array.array('I', map(int, output.read_bytes().split())).tobytes() == wanted
                                    hashes[name] = digest
                            if trial:
                                samples[name].append(float(elapsed))
                entry = dict(case=stem, operation=operation, input_sha256=sha(inp), answer_sha256=sha(answer), checksum=expected[0], count=int(expected[1]),
                             input_digit_histogram=histogram,
                             milliseconds=samples, median_ms={name:statistics.median(values) for name,values in samples.items()}, output_sha256=hashes)
                result['workloads'].append(entry)
                print(stem, operation, entry['median_ms'], flush=True)
        (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
