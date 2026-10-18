import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import tarfile
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
FLAGS = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--baseline', default='ce81108')
    p.add_argument('--candidate', type=Path, default=ROOT / 'fps/bostan_mori.hpp')
    p.add_argument('--experimental', choices=['fused_o3', 'tiled_o3', 'fused_o2', 'tiled_o2'])
    p.add_argument('--reference', required=True, type=Path)
    p.add_argument('--official-root', required=True, type=Path)
    p.add_argument('--output', required=True, type=Path)
    p.add_argument('--cpu', type=int, default=2)
    p.add_argument('--trials', type=int, default=11)
    args = p.parse_args()
    args.output = args.output.resolve()
    meta = {
        'retrieved_at': datetime.now(timezone.utc).isoformat(),
        'baseline': subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
        'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
        'cpu': args.cpu,
        'cpu_model': next(s.split(':', 1)[1].strip() for s in Path('/proc/cpuinfo').read_text().splitlines() if s.startswith('model name')),
        'flags': FLAGS,
        'harness_sha256': sha(Path(__file__)),
        'candidate_sha256': sha(args.candidate),
        'reference_sha256': sha(args.reference),
        'reference_submission': 'https://judge.yosupo.jp/submission/405848',
        'scope': 'baseline tree plus only the selected bostan_mori.hpp; whole-process wall time, regular-file stdin; one warmup then shuffled fresh processes',
        'trials': args.trials,
        'results': [],
        'binaries': {},
        'official_checks': {},
    }
    with tempfile.TemporaryDirectory(prefix='fps-bostan-') as tmp:
        w = Path(tmp)
        before, after = w / 'before', w / 'after'
        before.mkdir()
        archive = subprocess.check_output(['git', 'archive', args.baseline], cwd=ROOT)
        with tarfile.open(fileobj=io.BytesIO(archive)) as tf:
            tf.extractall(before, filter='data')
        shutil.copytree(before, after)
        shutil.copyfile(args.candidate, after / 'fps/bostan_mori.hpp')
        if args.experimental:
            header = (ROOT / 'test/benchmark/fps_bostan_transform_candidate.hpp').read_text()
            if args.experimental.startswith('fused'):
                first = header.index('\t\tint tile = std::min(n, 2048);')
                last = header.index('\t\tif (4 * h == n)', first)
                header = header[:first] + '\t\tfor (int j = 0; j < n; j += 8) {\n\t\t\tNTT::I8 z = NTT::load8(a + j);\n\t\t\tz = t.small8<4, true>(t.small8<2, true>(t.small8<1, true>(z, j / 8), j / 8), j / 8);\n\t\t\tNTT::store8(a + j, z);\n\t\t}\n\t\tint h = 8;\n\t\tfor (; 4 * h < n; h <<= 2) t.radix4_8<true>(a, h, n);\n' + header[last:]
            if args.experimental.endswith('o2'):
                header = header.replace('#pragma GCC optimize("O3")', '#pragma GCC optimize("O2")')
            (after / 'components/fps_bostan_transform.hpp').write_text(header)
            candidate = (before / 'fps/bostan_mori.hpp').read_text()
            candidate = candidate.replace('#include "fps.hpp"', '#include "fps.hpp"\n#include "../components/fps_bostan_transform.hpp"')
            old = 'plan.template fft<true>(odd, n);\n\t\t\todd[0] = fps_detail::NTT::sub(odd[0], (T(2 * n) * leading).v);\n\t\t\tfps_detail::NTT::product(odd, odd, O::words(powers), n);'
            assert candidate.count(old) == 1
            candidate = candidate.replace(old, 'fps_detail::BostanTransform{plan}.inverse_twisted(odd, n, O::words(powers), (T(2 * n) * leading).v);')
            if args.experimental.startswith('tiled'):
                candidate = candidate.replace('plan.template fft<false>(odd, n);', 'fps_detail::BostanTransform{plan}.forward(odd, n);')
            (after / 'fps/bostan_mori.hpp').write_text(candidate)
            meta['experimental'] = args.experimental
            meta['experimental_component_sha256'] = sha(after / 'components/fps_bostan_transform.hpp')
        meta['compiled_candidate_sha256'] = sha(after / 'fps/bostan_mori.hpp')
        sources = {
            'before_bostan': (before / 'test/fps_bostan_mori.test.cpp', before),
            'after_bostan': (after / 'test/fps_bostan_mori.test.cpp', after),
            'before_recurrence': (before / 'test/fps_linear_recurrence.test.cpp', before),
            'after_recurrence': (after / 'test/fps_linear_recurrence.test.cpp', after),
            'reference': (args.reference.resolve(), before),
        }
        with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            for name, (src, inc) in sources.items():
                binary = w / name
                subprocess.run(['g++', *FLAGS, '-I' + str(inc), str(src), '-o', str(binary)], check=True)
                meta['binaries'][name] = sha(binary)
            for name in ('after_bostan', 'after_recurrence'):
                count = 0
                for inp in sorted((args.official_root / 'in').glob('*.in')):
                    out = w / 'answer.out'
                    with inp.open('rb') as i, out.open('wb') as o:
                        subprocess.run([str(w / name)], stdin=i, stdout=o, check=True)
                    subprocess.run([str(args.official_root / 'checker'), str(inp), str(out), str(args.official_root / 'out' / (inp.stem + '.out'))], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    count += 1
                meta['official_checks'][name] = count
            fcntl.flock(lock, fcntl.LOCK_UN)
            os.sched_setaffinity(0, {args.cpu})
            fcntl.flock(lock, fcntl.LOCK_EX)
            rng = random.Random(20261019)
            for case in ('max_random_00', 'near_65536_00'):
                inp = args.official_root / 'in' / (case + '.in')
                expected = (args.official_root / 'out' / (case + '.out')).read_text().split()
                samples = {name: [] for name in sources}
                for trial in range(args.trials + 1):
                    order = list(sources)
                    rng.shuffle(order)
                    for name in order:
                        with inp.open('rb') as i:
                            start = time.perf_counter_ns()
                            result = subprocess.run([str(w / name)], stdin=i, stdout=subprocess.PIPE, check=True)
                            elapsed = (time.perf_counter_ns() - start) / 1e6
                        assert result.stdout.decode().split() == expected, (name, case)
                        if trial:
                            samples[name].append(elapsed)
                row = {'input': case, 'input_sha256': sha(inp), 'samples_ms': samples, 'medians_ms': {k: statistics.median(v) for k, v in samples.items()}}
                meta['results'].append(row)
                print(case, row['medians_ms'], flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(meta, indent=2) + '\n')


if __name__ == '__main__':
    main()
