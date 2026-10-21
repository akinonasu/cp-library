import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
PROBLEMS = [
    ('convolution', 'convolution_mod'),
    ('inv', 'inv_of_formal_power_series'),
    ('log', 'log_of_formal_power_series'),
    ('exp', 'exp_of_formal_power_series'),
    ('pow', 'pow_of_formal_power_series'),
    ('sqrt', 'sqrt_of_formal_power_series'),
    ('divmod', 'division_of_polynomials'),
    ('taylor_shift', 'polynomial_taylor_shift'),
    ('linear_recurrence', 'kth_term_of_linearly_recurrent_sequence'),
    ('multipoint_eval', 'multipoint_evaluation'),
    ('interpolate', 'polynomial_interpolation'),
]
QUOTE = re.compile(r'^\s*#\s*include\s*"([^"\r\n]+)"', re.MULTILINE)
GENERIC = r'''
#include "fps/fps_light_algorithms.hpp"
#include <atcoder/modint>
#include <cassert>
#include <random>
using M = atcoder::modint998244353;
using F = LightFormalPowerSeries<M>;
namespace A = light_fps_algorithms;
void equal(const F &a, const light_fps &b) {
    assert(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) assert(a[i].val() == int(b[i].val()));
}
int main() {
    std::mt19937 rng(756183);
    for (int n : {0, 1, 3, 33, 65, 129, 257, 1025, 4098}) {
        F a(n), b(n);
        light_fps x(n), y(n);
        std::vector<M> xs(n);
        std::vector<mint> ys(n);
        for (int i = 0; i < n; ++i) {
            unsigned av = rng() % mint::mod(), bv = rng() % mint::mod();
            a[i] = av; x[i] = av; b[i] = bv; y[i] = bv;
            xs[i] = i + 1; ys[i] = i + 1;
        }
        equal(a * b, x * y);
        equal(F::multiply(a, a), light_fps::multiply(x, x));
        if (n) a[0] = 7, x[0] = 7;
        equal(a.inv(), x.inv());
        if (n) a[0] = 1, x[0] = 1;
        equal(a.log(), x.log());
        if (n) a[0] = 0, x[0] = 0;
        equal(a.exp(), x.exp());
        equal(a.pow(5), x.pow(5));
        equal((a * a).sqrt(n), (x * x).sqrt(n));
        equal(A::taylor_shift(a, M(17)), A::taylor_shift(x, mint(17)));
        auto av = A::multipoint_eval(a, xs);
        auto xv = A::multipoint_eval(x, ys);
        assert(av.size() == xv.size());
        for (std::size_t i = 0; i < av.size(); ++i) assert(av[i].val() == int(xv[i].val()));
        equal(A::interpolate(xs, av), A::interpolate(ys, xv));
    }
}
'''
COEXIST = r'''
#if LIGHT_FIRST
#include "fps/fps_light_algorithms.hpp"
#endif
#include "fps/fps.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/taylor_shift.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#if !LIGHT_FIRST
#include "fps/fps_light_algorithms.hpp"
#endif
#include <cassert>
#include <random>
void equal(const light_fps &a, const fps &b) {
    assert(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) assert(a[i] == b[i]);
}
int main() {
    std::mt19937 rng(173833);
    for (int n : {0, 1, 3, 33, 65, 129, 257, 1025}) {
        light_fps a(n), b(n);
        fps x(n), y(n);
        for (int i = 0; i < n; ++i) { a[i] = x[i] = mint(rng()); b[i] = y[i] = mint(rng()); }
        equal(a * b, x * y);
        equal(a * a, x * x);
        if (n) a[0] = x[0] = 7;
        equal(a.inv(), x.inv());
        if (n) a[0] = x[0] = 1;
        equal(a.log(), x.log());
        if (n) a[0] = x[0] = 0;
        equal(a.exp(), x.exp());
        equal(a.pow(5), x.pow(5));
        if (n) a[0] = x[0] = 289;
        equal(a.sqrt(), x.sqrt());
        equal(light_fps_algorithms::taylor_shift(a, mint(17)), taylor_shift(x, mint(17)));
        std::vector<mint> xs(n);
        for (int i = 0; i < n; ++i) xs[i] = i;
        auto av = light_fps_algorithms::multipoint_eval(a, xs), xv = multipoint_eval(x, xs);
        assert(av == xv);
        equal(light_fps_algorithms::interpolate(xs, av), interpolate(xs, xv));
    }
}
'''


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def dependencies(path, seen):
    path = path.resolve()
    if path in seen:
        return
    seen.add(path)
    for match in QUOTE.finditer(path.read_text()):
        target = path.parent / match[1]
        if not target.is_file():
            target = ROOT / match[1]
        if not target.is_file():
            raise FileNotFoundError((path, match[1]))
        dependencies(target, seen)


def run(args):
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    flags = ['-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror', '-I', str(ROOT)]
    headers = sorted((ROOT / 'fps').glob('fps_light*.hpp')) + sorted((ROOT / 'components').glob('fps_light*.hpp'))
    units = sorted((ROOT / 'test/unit').glob('fps_light*.cpp'))
    seen = set()
    for path in headers + units + [ROOT / ('test/fps_light_' + name + '.test.cpp') for name, _ in PROBLEMS]:
        dependencies(path, seen)
    heavy = set()
    for path in ['fps/fps.hpp', 'fps/linear_recurrence.hpp', 'fps/taylor_shift.hpp', 'fps/multipoint_eval.hpp', 'fps/interpolate.hpp']:
        dependencies(ROOT / path, heavy)
    seen |= heavy
    report = {
        'started_at': datetime.now(timezone.utc).isoformat(),
        'revision': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
        'baseline_commit': subprocess.check_output(['git', 'rev-parse', args.baseline], cwd=ROOT, text=True).strip(),
        'script_sha256': sha(Path(__file__)),
        'source_sha256': {p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(seen)},
        'compiler_versions': {cxx: subprocess.check_output([cxx, '--version'], text=True) for cxx in args.cxx},
        'flags': flags, 'stages': args.stages, 'standalone': [], 'unit': [], 'generic': [], 'coexist': [], 'official': [], 'completed': False,
    }
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    resource.setrlimit(resource.RLIMIT_STACK, (hard, hard))
    report['stack_limit'] = list(resource.getrlimit(resource.RLIMIT_STACK))
    for path in sorted(heavy):
        name = path.relative_to(ROOT).as_posix()
        old = subprocess.check_output(['git', 'show', report['baseline_commit'] + ':' + name], cwd=ROOT)
        assert hashlib.sha256(old).hexdigest() == sha(path), 'Existing heavy FPS changed: ' + name
    report['existing_heavy_headers_unchanged'] = sorted(p.relative_to(ROOT).as_posix() for p in heavy)

    def save():
        output.write_text(json.dumps(report, indent=2) + '\n')

    save()
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock, tempfile.TemporaryDirectory(prefix='fps-light-check-') as directory:
        fcntl.flock(lock, fcntl.LOCK_SH)
        work = Path(directory)

        def compile_run(cxx, source, label, extra=(), sanitize=False):
            executable = work / label
            command = [cxx, *flags, *extra]
            if sanitize:
                command += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
            command += [str(source), '-o', str(executable)]
            subprocess.run(command, cwd=ROOT, check=True)
            environment = os.environ.copy()
            if sanitize:
                environment['UBSAN_OPTIONS'] = 'halt_on_error=1'
            executed = subprocess.run([str(executable)], cwd=ROOT, capture_output=True, text=True, check=True, timeout=args.timeout, env=environment)
            return {'compiler': cxx, 'source_sha256': sha(source), 'command': command, 'binary_sha256': sha(executable), 'stdout': executed.stdout, 'stderr': executed.stderr}

        if 'standalone' in args.stages:
            for cxx in args.cxx:
                for path in headers:
                    name = path.relative_to(ROOT).as_posix()
                    source = work / 'header.cpp'
                    source.write_text('#include "' + name + '"\n#include "' + name + '"\n'
                                      '#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) || defined(all) || defined(sz) || defined(uint)\n'
                                      '#error user_macro_dependency\n#endif\nint main() {}\n')
                    command = [cxx, *flags, '-DNDEBUG', '-fsyntax-only', str(source)]
                    subprocess.run(command, cwd=ROOT, check=True)
                    report['standalone'].append({'compiler': cxx, 'header': name, 'command': command})
                save()
        for stage in ['unit', 'sanitize']:
            if stage not in args.stages:
                continue
            compilers = args.cxx if stage == 'unit' else [args.sanitize_cxx]
            for cxx in compilers:
                for source in units:
                    row = compile_run(cxx, source, source.stem, sanitize=stage == 'sanitize')
                    row.update({'source': source.relative_to(ROOT).as_posix(), 'sanitized': stage == 'sanitize'})
                    report['unit'].append(row)
                    save()
                    print(stage, cxx, source.name, 'passed', flush=True)
        if 'generic' in args.stages:
            source = work / 'generic.cpp'
            source.write_text(GENERIC)
            report['acl_commit'] = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.acl, text=True).strip()
            for cxx in args.cxx:
                report['generic'].append(compile_run(cxx, source, 'generic', extra=['-isystem', str(args.acl)]))
                save()
                print('generic', cxx, 'passed', flush=True)
        if 'coexist' in args.stages:
            source = work / 'coexist.cpp'
            source.write_text(COEXIST)
            for cxx in args.cxx:
                for order in [0, 1]:
                    row = compile_run(cxx, source, 'coexist', extra=['-DLIGHT_FIRST=' + str(order)])
                    row['light_first'] = bool(order)
                    report['coexist'].append(row)
                    save()
                    print('coexist', cxx, order, 'passed', flush=True)
        if 'official' in args.stages:
            report['generator_commit'] = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=args.problems, text=True).strip()
            report['cases'] = {}
            for name, problem in PROBLEMS:
                found = list(args.problems.glob('*/' + problem + '/info.toml'))
                assert len(found) == 1, (problem, found)
                directory = found[0].parent
                inputs = sorted((directory / 'in').glob('*.in'))
                assert inputs and (directory / 'checker').is_file(), 'Generate official cases first: ' + problem
                report['cases'][name] = {'problem': problem, 'checker_sha256': sha(directory / 'checker'),
                                         'inputs': {case.name: {'input_sha256': sha(case), 'answer_sha256': sha(directory / 'out' / (case.stem + '.out'))} for case in inputs}}
                for cxx in args.cxx:
                    source = ROOT / ('test/fps_light_' + name + '.test.cpp')
                    executable = work / name
                    command = [cxx, *flags, str(source), '-o', str(executable)]
                    subprocess.run(command, cwd=ROOT, check=True)
                    for case in inputs:
                        actual = work / 'actual.out'
                        with case.open('rb') as inp, actual.open('wb') as out:
                            subprocess.run([str(executable)], stdin=inp, stdout=out, cwd=ROOT, timeout=args.timeout, check=True)
                        expected = directory / 'out' / (case.stem + '.out')
                        checked = subprocess.run([str(directory / 'checker'), str(case), str(actual), str(expected)], capture_output=True, text=True, timeout=args.timeout)
                        assert checked.returncode == 0, (name, cxx, case.name, checked.stdout, checked.stderr)
                    report['official'].append({'operation': name, 'driver': source.relative_to(ROOT).as_posix(), 'compiler': cxx, 'cases_passed': len(inputs), 'binary_sha256': sha(executable), 'command': command})
                    save()
                    print('official', name, cxx, len(inputs), 'passed', flush=True)
    for name, expected in report['source_sha256'].items():
        assert sha(ROOT / name) == expected, 'Source changed while checking: ' + name
    report.update(completed=True, completed_at=datetime.now(timezone.utc).isoformat())
    save()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--problems', type=Path)
    parser.add_argument('--acl', type=Path)
    parser.add_argument('--cxx', nargs='+', default=['g++', 'clang++'])
    parser.add_argument('--sanitize-cxx', default='clang++')
    parser.add_argument('--baseline', default='5e7f6ec')
    parser.add_argument('--stages', nargs='+', choices=['unit', 'sanitize', 'standalone', 'generic', 'coexist', 'official'], default=['unit', 'sanitize', 'standalone', 'generic', 'coexist', 'official'])
    parser.add_argument('--timeout', type=int, default=120)
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_light_speed/validation.json')
    args = parser.parse_args()
    if 'official' in args.stages and args.problems is None:
        parser.error('--problems is required for official checks')
    if 'generic' in args.stages and args.acl is None:
        parser.error('--acl is required for generic coefficient checks')
    if args.problems:
        args.problems = args.problems.resolve()
    if args.acl:
        args.acl = args.acl.resolve()
    run(args)


if __name__ == '__main__':
    main()
