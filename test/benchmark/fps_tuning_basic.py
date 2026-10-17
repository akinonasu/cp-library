import argparse
import fcntl
import hashlib
import json
from pathlib import Path
import random
import re
import statistics
import subprocess
import tempfile
from contextlib import contextmanager


ROOT = Path(__file__).resolve().parents[2]
FILES = ('fps/fps.hpp', 'components/fps_operations.hpp', 'components/ntt.hpp', 'math/mint.hpp')


@contextmanager
def exclusive():
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        yield


def wide(text, function, count):
    begin = text.index('\n\tF ' + function + '(')
    end = text.find('\n\tF ', begin + 1)
    if end < 0:
        end = len(text)
    body = text[begin:end]
    old = 3 if function == 'exp' else 4 if function == 'div_series' else 6
    body = body.replace(f'j += {old}', f'j += {count}').replace(f'[{old}]', f'[{count}]').replace(f'std::min({old},', f'std::min({count},')
    match = re.search(r'(?m)^(\t+)if \(terms == ' + str(old) + r'\) update', body)
    assert match
    indent = match.group(1)
    dispatch_begin = match.start()
    dispatch_end = body.index('\n' + indent[:-1] + '}', dispatch_begin)
    dispatch = '\n'.join(f'{indent}{"if" if k == count else "else if"} (terms == {k}) update(std::integral_constant<int, {k}>{{}});' for k in range(count, 1, -1))
    dispatch += '\n' + indent + 'else update(std::integral_constant<int, 1>{});'
    body = body[:dispatch_begin] + dispatch + body[dispatch_end:]
    if function == 'exp':
        body = body.replace('\t\t\t\tstd::fill(error.begin(), error.end(), T(0));\n', '')
        old_update = '\t\t\t\t\t\tfps_detail::NTT::template accumulate_batch<count>(words(error), a, b, c, block, false);\n\t\t\t\t\t\tfps_detail::NTT::template accumulate_batch<count>(words(error) + block, a, b, c, block, true);'
        new_update = '\t\t\t\t\t\tif (j == 0) {\n\t\t\t\t\t\t\tfps_detail::NTT::template accumulate_wide<count, true, false>(words(error), a, b, c, block, false);\n\t\t\t\t\t\t\tfps_detail::NTT::template accumulate_wide<count, true, false>(words(error) + block, a, b, c, block, true);\n\t\t\t\t\t\t} else {\n\t\t\t\t\t\t\tfps_detail::NTT::template accumulate_wide<count, false, false>(words(error), a, b, c, block, false);\n\t\t\t\t\t\t\tfps_detail::NTT::template accumulate_wide<count, false, false>(words(error) + block, a, b, c, block, true);\n\t\t\t\t\t\t}'
        assert old_update in body
        body = body.replace(old_update, new_update)
    return text[:begin] + body + text[end:]


def window_group(text, count):
    for function in ('div_series', 'exp'):
        actual = 'exp_impl' if function == 'exp' and '\n\tF exp_impl(' in text else function
        begin = text.index('\n\tF ' + actual + '(')
        end = text.find('\n\tF ', begin + 1)
        if end < 0:
            end = text.index('\n\tstd::vector', begin)
        body = text[begin:end]
        body = body.replace('j += 7', f'j += {count}').replace('[7]', f'[{count}]').replace('std::min(7,', f'std::min({count},')
        match = re.search(r'(?m)^(\t+)if \(terms == 7\) update', body)
        assert match
        indent = match.group(1)
        dispatch_end = body.index('\n' + indent[:-1] + '}', match.start())
        dispatch = '\n'.join(f'{indent}{"if" if k == count else "else if"} (terms == {k}) update(std::integral_constant<int, {k}>{{}});' for k in range(count, 1, -1))
        dispatch += '\n' + indent + 'else update(std::integral_constant<int, 1>{});'
        body = body[:match.start()] + dispatch + body[dispatch_end:]
        text = text[:begin] + body + text[end:]
    return text


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--trials', type=int, default=7)
    p.add_argument('--sizes', type=int, nargs='+', default=[32768, 524288])
    p.add_argument('--variants', nargs='+')
    p.add_argument('--operations', nargs='+', default=['inv', 'log', 'exp', 'sqrt', 'pow', 'div_series', 'square', 'divmod'])
    args = p.parse_args()
    snapshot = {name: (ROOT / name).read_bytes() for name in FILES}
    current = snapshot[FILES[0]].decode().replace('\r\n', '\n')
    source = subprocess.check_output(['git', 'show', 'fd41ae5df33960b93288ae98f9edac4e9fdb166f:fps/fps.hpp'], cwd=ROOT, text=True)
    variants = {'before': source}
    for count in (4, 8, 16, 64, 128):
        variants[f'block{count}'] = source.replace('const int block = size / 32', f'const int block = size / {count}')
    for count in (4, 6, 7):
        variants[f'exp_wide{count}'] = wide(source, 'exp', count)
    for count in (6, 7):
        variants[f'div_wide{count}'] = wide(source, 'div_series', count)
    variants['sqrt_wide7'] = wide(source, 'sqrt', 7)
    variants['inv_div'] = source.replace('if (deg < 1024 || !fps_detail::NTT::vectorized())', 'if (true)')
    variants['square_conv'] = source.replace('if (&a != &b) {', 'if (true) {', 1).replace('if (n >= 64 && &a != &b)', 'if (n >= 64)', 1)
    variants['exp_wide7_block16'] = variants['exp_wide7'].replace('const int block = size / 32', 'const int block = size / 16')
    variants['newton'] = source.replace('if (deg > 256) {', 'if (false) {').replace('if (target > 256) {', 'if (false) {')
    variants['windows'] = current
    variants['windows_block16'] = current.replace('const int block = size / 32', 'const int block = size / 16')
    variants['windows_block64'] = current.replace('const int block = size / 32', 'const int block = size / 64')
    for count in (3, 4, 6):
        variants[f'windows_group{count}'] = window_group(current, count)
    variants['windows_floor'] = current.replace('while (size < deg) size <<= 1;', 'while (size * 2 <= deg) size <<= 1;').replace('while (size < target) size <<= 1;', 'while (size * 2 <= target) size <<= 1;')
    variants['windows_nearest'] = current.replace('while (size < deg) size <<= 1;', 'while (size < deg) size <<= 1; if (deg < size - size / 4) size >>= 1;').replace('while (size < target) size <<= 1;', 'while (size < target) size <<= 1; if (target < size - size / 4) size >>= 1;')
    variants['pow_unfused'] = current.replace('f = f.log_derivative(len);', 'f /= lead; f = f.log(len);').replace('f = f.template exp_impl<true>(len);', 'f = f.exp(len);')
    if '\n\t\tint cyclic = 1;' in current:
        begin = current.index('\n\t\tint cyclic = 1;')
        end = current.index('\n\t\tF product = multiply(rb.pre', begin)
        variants['divmod_full'] = current[:begin] + current[end:]
    if args.variants:
        variants = {key: variants[key] for key in args.variants}
    flags = ['-std=c++23', '-O2', '-DNDEBUG', '-march=native', '-ftrivial-auto-var-init=zero']
    environment = {'flags': flags, 'compiler': subprocess.check_output(['g++', '--version'], text=True).splitlines()[0], 'cpu': subprocess.check_output(['lscpu'], text=True), 'cpu_affinity': 2, 'trials': args.trials, 'source_sha256': {name: hashlib.sha256(snapshot[name]).hexdigest() for name in FILES}, 'seed': 731, 'variants': list(variants)}
    environment['variant_header_sha256'] = {name: hashlib.sha256(header.encode()).hexdigest() for name, header in variants.items()}
    environment['driver_sha256'] = hashlib.sha256((ROOT / 'test/benchmark/fps_tuning_basic.cpp').read_bytes()).hexdigest()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    results = []
    with tempfile.TemporaryDirectory(prefix='fps-tuning-basic-') as td:
        temp = Path(td)
        for variant, header in variants.items():
            directory = temp / variant
            for name in FILES:
                (directory / name).parent.mkdir(parents=True, exist_ok=True)
                (directory / name).write_bytes(snapshot[name])
            (directory / FILES[0]).write_text(header)
            command = ['g++', *flags, '-I', str(directory), f'-DFPS_TUNING_HEADER="{directory / FILES[0]}"', str(ROOT / 'test/benchmark/fps_tuning_basic.cpp'), '-o', str(temp / (variant + '.out'))]
            with exclusive():
                subprocess.run(command, check=True)
            print('built', variant, flush=True)
        order = random.Random(403023)
        for n in args.sizes:
            for op in args.operations:
                checksums = set()
                samples = {v: [] for v in variants}
                with exclusive():
                    for trial in range(args.trials + 1):
                        names = list(variants)
                        order.shuffle(names)
                        for variant in names:
                            output = subprocess.check_output(['taskset', '-c', '2', str(temp / (variant + '.out')), op, str(n)], text=True).split()
                            checksums.add(output[1])
                            if trial:
                                samples[variant].append(float(output[0]))
                assert len(checksums) == 1, (n, op, checksums)
                for variant in variants:
                    results.append({'size': n, 'operation': op, 'variant': variant, 'median': statistics.median(samples[variant]), 'samples': samples[variant], 'checksum': next(iter(checksums))})
                print(n, op, {v: round(statistics.median(s) * 1000, 3) for v, s in samples.items()}, flush=True)
                args.output.write_text(json.dumps({'environment': environment, 'results': results}, indent=2) + '\n')


if __name__ == '__main__':
    main()
