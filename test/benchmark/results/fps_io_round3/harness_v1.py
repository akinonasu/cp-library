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


def variants(original):
    result = {'baseline': original}
    old = '\tstatic void read(FastInput &in, unsigned *out, std::size_t count) {\n\t\tstd::unique_ptr<unsigned[]> scratch;'
    new = '\tstatic void read(FastInput &in, unsigned *out, std::size_t count, std::unique_ptr<unsigned[]> &scratch) {'
    assert original.count(old) == 1
    reuse = original.replace(old, new)
    end = '\n};\nstruct BulkOutput'
    reuse = reuse.replace(end, '\n\tstatic void read(FastInput &in, unsigned *out, std::size_t count) {\n\t\tstd::unique_ptr<unsigned[]> scratch;\n\t\tread(in, out, count, scratch);\n\t}\n' + end)
    reuse = reuse.replace('\tauto values = std::make_unique_for_overwrite<unsigned[]>', '\tstd::unique_ptr<unsigned[]> scratch;\n\tauto values = std::make_unique_for_overwrite<unsigned[]>')
    reuse = reuse.replace('BulkInput::read(input, values.get(), count);', 'BulkInput::read(input, values.get(), count, scratch);')
    result['reuse'] = reuse
    begin = original.index('\tstatic std::size_t chunk(')
    stop = original.index('\n\tstatic void read(FastInput', begin)
    chunk = original[begin:stop]
    chunk = chunk.replace('\tstatic std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch)', '\ttemplate <class Emit>\n\tstatic std::size_t chunk_blocks(const char *&p, const char *limit, std::size_t count, unsigned *scratch, Emit emit)')
    chunk = chunk.replace('{dst, scratch, scratch + kCapacity, scratch + 2 * kCapacity}', '{scratch, scratch + kCapacity, scratch + 2 * kCapacity, scratch + 3 * kCapacity}')
    a = chunk.index('\t\tauto *next = dst + n[0];')
    b = chunk.index('\t\tp = last;', a)
    chunk = chunk[:a] + '\t\tfor (int k = 0; k < 4; ++k) emit(out[k], n[k]);\n' + chunk[b:]
    reader = '''
	static std::size_t chunk(const char *&p, const char *limit, unsigned *dst, std::size_t count, unsigned *scratch) {
		std::size_t offset = 0;
		return chunk_blocks(p, limit, count, scratch, [&](const unsigned *values, std::size_t n) {
			std::memcpy(dst + offset, values, n * sizeof(unsigned));
			offset += n;
		});
	}

	template <class Emit>
	static void read_blocks(FastInput &in, std::size_t count, Emit emit) {
		std::unique_ptr<unsigned[]> scratch;
		std::size_t done = 0;
		while (done < count) {
			const char *p = in.current;
			while (p != in.end && static_cast<unsigned char>(*p) <= ' ') ++p;
			while (count - done >= 1024 && in.end - p >= 1088) {
				if (!scratch) scratch = std::make_unique_for_overwrite<unsigned[]>(4 * kCapacity);
				const auto parsed = chunk_blocks(p, in.end, count - done, scratch.get(), emit);
				if (!parsed) break;
				done += parsed;
			}
			while (done + 2 <= count && in.end - p >= 32) {
				const char *next = p;
				auto invalid = _mm256_setzero_si256();
				const auto groups = two(next, invalid);
				if (!_mm256_testz_si256(invalid, invalid)) break;
				const auto halves = _mm256_madd_epi16(_mm256_packus_epi32(groups, groups), _mm256_set1_epi32(0x00012710));
				const unsigned h0 = _mm256_extract_epi32(halves, 0), l0 = _mm256_extract_epi32(halves, 1);
				const unsigned h1 = _mm256_extract_epi32(halves, 4), l1 = _mm256_extract_epi32(halves, 5);
				if (h0 > 42 || (h0 == 42 && l0 > 94967295) || h1 > 42 || (h1 == 42 && l1 > 94967295)) break;
				const unsigned values[2] = {h0 * 100000000 + l0, h1 * 100000000 + l1};
				emit(values, 2);
				done += 2;
				p = next;
			}
			in.current = p;
			if (done < count) {
				unsigned value = 0;
				in.read(value);
				emit(&value, 1);
				++done;
			}
		}
	}

	static void read(FastInput &in, unsigned *out, std::size_t count) {
		read_blocks(in, count, [&](const unsigned *values, std::size_t n) {
			std::memcpy(out, values, n * sizeof(unsigned));
			out += n;
		});
	}
'''
    reader = reader[reader.index('\n\ttemplate <class Emit>'):]
    end_reader = original.index('\n};\nstruct BulkOutput', stop)
    fused = original[:stop] + chunk + reader + original[end_reader:]
    begin_coeff = fused.index('\tauto values = std::make_unique_for_overwrite<unsigned[]>', fused.index('inline void read_coefficients'))
    end_coeff = fused.index('\n}\n\ninline void write_coefficients', begin_coeff)
    fused = fused[:begin_coeff] + '''	fast_io_detail::BulkInput::read_blocks(input, n, [&](const unsigned *values, std::size_t count) {
		append_fps_words<false>(out, values, count);
	});''' + fused[end_coeff:]
    result['fused'] = fused
    result['fused_small'] = fused.replace('kChunk = 1 << 17', 'kChunk = 1 << 15')
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--baseline-root', type=Path, default=Path('/tmp/cp-fps-round3-base'))
    parser.add_argument('--work', type=Path, default=Path('/tmp/fps-io-round3'))
    parser.add_argument('--official-root', type=Path, default=Path('/tmp/library-checker-problems'))
    parser.add_argument('--references', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--variants', nargs='+', default=['baseline', 'reuse', 'fused', 'fused_small'])
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--seed', type=int, default=20261029)
    parser.add_argument('--cases', nargs='+', default=['max_random_00', 'max_random_01'])
    args = parser.parse_args()
    args.work.mkdir(parents=True, exist_ok=True)
    source = (args.baseline_root / 'components/fps_io.hpp').read_text()
    candidates = {name: text for name, text in variants(source).items() if name in args.variants}
    flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
    problems = ['convolution_mod', 'log_of_formal_power_series']
    folders = {p.parent.name: p.parent for p in args.official_root.glob('*/*/info.toml')}
    drivers = {name: next(p for p in sorted((args.baseline_root / 'test').glob('*.test.cpp')) if f'"https://judge.yosupo.jp/problem/{name}"' in p.read_text()) for name in problems}
    result = dict(baseline='bd951dce6fdcb3ea9db4fcef342147012a8ab0c0', flags=flags,
                  compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0],
                  cpu=2, trials=args.trials, seed=args.seed, harness_sha256=sha(Path(__file__)),
                  hardware=subprocess.check_output(['lscpu'], text=True),
                  official_commit=subprocess.check_output(['git', '-C', str(args.official_root), 'rev-parse', 'HEAD'], text=True).strip(),
                  headers={str(p.relative_to(args.baseline_root)): sha(p) for folder in ('components', 'fps', 'math', 'convolution') for p in (args.baseline_root / folder).glob('*.hpp')}, sources={}, results=[])
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        jobs = []
        for name, text in candidates.items():
            header = args.work / (name + '.hpp')
            for include in ['fast_io.hpp', 'fps_output.hpp']:
                text = text.replace('#include "' + include + '"', '#include "' + str(args.baseline_root / 'components' / include) + '"')
            text = text.replace('#include "../math/mint.hpp"', '#include "' + str(args.baseline_root / 'math/mint.hpp') + '"')
            header.write_text(text)
            for problem in problems:
                binary = args.work / (problem + '_' + name)
                command = ['g++', *flags, '-I', str(args.baseline_root), '-include', str(header), str(drivers[problem]), '-o', str(binary)]
                jobs.append((problem, name, header, binary, command))
        for problem, reference in [('convolution_mod', 409465), ('log_of_formal_power_series', 409452)]:
            reference_source = args.references / problem / (str(reference) + '.cpp')
            binary = args.work / (problem + '_reference')
            jobs.append((problem, 'reference', None, binary, ['g++', *flags, str(reference_source), '-o', str(binary)]))

        def build(job):
            problem, name, header, binary, command = job
            subprocess.run(command, check=True)
            return problem + '_' + name, dict(command=command, header_sha256=sha(header) if header else None, source_sha256=sha(Path(command[-3])), binary_sha256=sha(binary))

        with ThreadPoolExecutor(max_workers=2) as pool:
            for key, data in pool.map(build, jobs):
                result['sources'][key] = data
                print('built', key, flush=True)
        fcntl.flock(lock, fcntl.LOCK_UN)
        fcntl.flock(lock, fcntl.LOCK_EX)
        os.sched_setaffinity(0, {2})
        rng = random.Random(args.seed)
        for problem in problems:
            for case in args.cases:
                folder = folders[problem]
                inp, answer = folder / 'in' / (case + '.in'), folder / 'out' / (case + '.out')
                if not inp.exists():
                    continue
                samples = {v: [] for v in [*candidates, 'reference']}
                for trial in range(args.trials + 1):
                    order = list(samples)
                    rng.shuffle(order)
                    for name in order:
                        output = args.work / (problem + '_' + case + '_' + name + '.out')
                        with inp.open('rb') as fin, output.open('wb') as fout:
                            start = time.perf_counter_ns()
                            subprocess.run([str(args.work / (problem + '_' + name))], stdin=fin, stdout=fout, check=True)
                            ms = (time.perf_counter_ns() - start) / 1e6
                        subprocess.run([str(folder / 'checker'), str(inp), str(output), str(answer)], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        if trial:
                            samples[name].append(ms)
                row = dict(problem=problem, case=case, input_sha256=sha(inp), answer_sha256=sha(answer), checker_sha256=sha(folder / 'checker'), variants={v: dict(median_ms=statistics.median(s), samples_ms=s, output_sha256=sha(args.work / (problem + '_' + case + '_' + v + '.out'))) for v, s in samples.items()})
                result['results'].append(row)
                args.output.parent.mkdir(parents=True, exist_ok=True)
                args.output.write_text(json.dumps(result, indent=2) + '\n')
                print(problem, case, {v: x['median_ms'] for v, x in row['variants'].items()}, flush=True)


if __name__ == '__main__':
    main()
