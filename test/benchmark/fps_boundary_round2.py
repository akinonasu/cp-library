import argparse
from concurrent.futures import ThreadPoolExecutor
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-root', type=Path, default=Path('/tmp/cp-fps-round2-base'))
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-round2-output'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--trials', type=int, default=15)
    parser.add_argument('--kind', choices=['output', 'input'], default='output')
    parser.add_argument('--variants', nargs='+')
    parser.add_argument('--problems', nargs='+')
    parser.add_argument('--seed', type=int, default=20261022)
    parser.add_argument('--cases', nargs='+', default=['max_random_00'])
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    header_name = 'fps_output.hpp' if args.kind == 'output' else 'fps_io.hpp'
    original = (args.baseline_root / 'components' / header_name).read_text()
    marker = 'template <bool montgomery>'
    variants = {'baseline': original}
    if args.kind == 'output':
        prefix, function = original.split(marker)
        for batch in [256, 1024]:
            variants['batch' + str(batch)] = prefix + marker + function.replace('64', str(batch))
        start = '\tstd::array<Mint, 64> batch;'
        end = '\n}\n}\n#pragma GCC pop_options'
        first, tail = original.split(start)
        _, last = tail.split(end)
        variants['resize'] = first + '''\tconst auto offset = out.size();
\tout.resize(offset + n);
\tif constexpr (montgomery) std::memcpy(static_cast<void *>(out.data() + offset), in, n * sizeof(Mint));
\telse {
\t\tstd::size_t i = 0;
\t\tfor (; i + 8 <= n; i += 8) {
\t\t\tconst auto x = fps_output_detail::montgomery(_mm256_loadu_si256(reinterpret_cast<const __m256i *>(in + i)));
\t\t\tstd::memcpy(static_cast<void *>(out.data() + offset + i), &x, sizeof(x));
\t\t}
\t\tfor (; i < n; ++i) out[offset + i] = Mint::raw(in[i]);
\t}''' + end + last
        variants['fill'] = variants['resize'].replace('out.resize(offset + n);', 'out.insert(out.end(), n, Mint::word(0));')
    else:
        for block in [131072, 524288]:
            variants['block' + str(block)] = original.replace('32768', str(block))
        variants['half_scratch'] = original.replace('kCapacity = kChunk / 4 + 66', 'kCapacity = kChunk / 8 + 66')
        begin = '\t\tconst auto control = _mm256_set_m128i('
        offset = original.index(begin)
        stop = original.index('\n\t\tconst auto digits', offset)
        variants['control'] = original[:offset] + '''\t\tconst auto lengths = _mm256_set1_epi32(a | ((b - a - 1) << 8));
\t\tconst auto lanes = _mm256_set_m128i(_mm_set1_epi8(1), _mm_setzero_si128());
\t\tconst auto indices = _mm256_broadcastsi128_si256(_mm_setr_epi8(-16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1));
\t\tconst auto control = _mm256_add_epi8(_mm256_shuffle_epi8(lengths, lanes), indices);''' + original[stop:]
        variants['step22'] = original.replace('p += std::min(b + 1, 32u);', 'p += std::min(b + 1, 22u);').replace('left / 33', '(left >= 32 ? (left - 10) / 22 : 0)')
        prepare = '''\tstatic void prepare(FastInput &in, std::size_t n) {
#if defined(MADV_POPULATE_READ)
\t\tif (n < 131072 || !in.mapping) return;
\t\tconst auto available = std::size_t(in.end - in.current);
\t\tconst auto bytes = n < available / 11 ? 11 * n : available;
\t\tconst auto first = reinterpret_cast<std::uintptr_t>(in.current);
\t\tconst auto lo = (first + 4095) & ~std::uintptr_t(4095), hi = (first + bytes) & ~std::uintptr_t(4095);
\t\tif (lo < hi) static_cast<void>(madvise(reinterpret_cast<void *>(lo), hi - lo, MADV_POPULATE_READ));
#endif
\t}

'''
        variants['populate'] = original.replace('\tstatic void read(FastInput', prepare + '\tstatic void read(FastInput').replace('prepare_fps_output(out, out.size() + n);', 'prepare_fps_output(out, out.size() + n);\n\tfast_io_detail::BulkInput::prepare(input, n);')
        variants['block_populate'] = variants['populate'].replace('32768', '131072')
        variants['block_step22'] = variants['step22'].replace('32768', '131072')
        variants['block_populate_step22'] = variants['block_populate'].replace('p += std::min(b + 1, 32u);', 'p += std::min(b + 1, 22u);').replace('left / 33', '(left >= 32 ? (left - 10) / 22 : 0)')
        variants['populate_step22'] = variants['populate'].replace('p += std::min(b + 1, 32u);', 'p += std::min(b + 1, 22u);').replace('left / 33', '(left >= 32 ? (left - 10) / 22 : 0)')
        variants['populate_large'] = variants['populate'].replace('const auto first = reinterpret_cast<std::uintptr_t>(in.current);', 'if (bytes < (1u << 22)) return;\n\t\tconst auto first = reinterpret_cast<std::uintptr_t>(in.current);')
    if args.variants:
        variants = {v: variants[v] for v in args.variants}
    drivers = {
        'convolution_mod': 'test/convolution_mod.test.cpp',
        'log_of_formal_power_series': 'test/log_of_formal_power_series.test.cpp',
    }
    for problem in drivers:
        matches = sorted((args.baseline_root / 'test').glob('*.test.cpp'))
        drivers[problem] = next(str(p.relative_to(args.baseline_root)) for p in matches if 'https://judge.yosupo.jp/problem/' + problem + '"' in p.read_text())
    if args.problems:
        drivers = {p: drivers[p] for p in args.problems}
    folders = {p.parent.name: p.parent for p in args.official_root.glob('*/*/info.toml')}
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    result = dict(compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0], flags=flags,
                  hardware=subprocess.check_output(['lscpu'], text=True), cpu=2, trials=args.trials,
                  kind=args.kind, seed=args.seed, harness_sha256=sha(Path(__file__)), original_header_sha256=sha(args.baseline_root / 'components' / header_name),
                  baseline_headers={str(p.relative_to(args.baseline_root)): sha(p) for folder in ['components', 'fps', 'math', 'convolution'] for p in (args.baseline_root / folder).glob('*.hpp')},
                  official_commit=subprocess.check_output(['git', '-C', str(args.official_root), 'rev-parse', 'HEAD'], text=True).strip(),
                  sources={}, results=[])
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        jobs = []
        for variant, source in variants.items():
            header = args.work / (variant + '.hpp')
            source = source.replace('#include "../math/mint.hpp"', '#include "' + str(args.baseline_root / 'math/mint.hpp') + '"')
            for name in ['fast_io.hpp', 'fps_output.hpp']:
                source = source.replace('#include "' + name + '"', '#include "' + str(args.baseline_root / 'components' / name) + '"')
            header.write_text(source)
            for problem, relative in drivers.items():
                binary = args.work / (problem + '_' + variant)
                command = ['g++', *flags, '-I', str(args.baseline_root), '-include', str(header), str(args.baseline_root / relative), '-o', str(binary)]
                jobs.append((problem, variant, binary, command, header))

        def build(job):
            problem, variant, binary, command, header = job
            subprocess.run(command, check=True)
            return problem + '_' + variant, dict(command=command, header_sha256=sha(header), source_sha256=sha(args.baseline_root / drivers[problem]), binary_sha256=sha(binary))

        with ThreadPoolExecutor(max_workers=2) as pool:
            for key, data in pool.map(build, jobs):
                result['sources'][key] = data
                print('built', key, flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        rng = random.Random(args.seed)
        for problem, case in [(p, c) for p in drivers for c in args.cases if (folders[p] / ('in/' + c + '.in')).exists()]:
            folder = folders[problem]
            inp, answer = folder / ('in/' + case + '.in'), folder / ('out/' + case + '.out')
            samples = {v: [] for v in variants}
            for trial in range(args.trials + 1):
                order = list(variants)
                rng.shuffle(order)
                for variant in order:
                    output = args.work / (problem + '_' + case + '_' + variant + '.out')
                    with inp.open('rb') as fin, output.open('wb') as fout:
                        begin = time.perf_counter_ns()
                        subprocess.run([str(args.work / (problem + '_' + variant))], stdin=fin, stdout=fout, check=True)
                        elapsed = (time.perf_counter_ns() - begin) / 1e6
                    subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    if trial:
                        samples[variant].append(elapsed)
            row = dict(problem=problem, case=case, input_sha256=sha(inp), answer_sha256=sha(answer), checker_sha256=sha(folder / 'checker'), variants={v: dict(median_ms=statistics.median(s), samples_ms=s, output_sha256=sha(args.work / (problem + '_' + case + '_' + v + '.out'))) for v, s in samples.items()})
            result['results'].append(row)
            print(problem, case, {v: x['median_ms'] for v, x in row['variants'].items()}, flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
