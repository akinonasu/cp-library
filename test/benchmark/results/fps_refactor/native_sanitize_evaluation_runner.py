import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[2]
UNITS = [
    'test/unit/fps_convolution_borrow.cpp',
    'test/unit/fps_convolution.cpp',
    'test/unit/fps_convolution_constant.cpp',
    'test/unit/fps_composition_blocked.cpp',
    'test/unit/fps_evaluation.cpp',
    'test/unit/fps_tuning_basic.cpp',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cxx', default='g++')
    parser.add_argument('--validation', type=Path, default=ROOT / 'test/benchmark/results/fps_refactor/validation.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_refactor/native_sanitize.json')
    parser.add_argument('--work', type=Path, default=Path('/workspace/cp-fps-refactor-validation'))
    parser.add_argument('--units', nargs='+', choices=UNITS, default=UNITS)
    parser.add_argument('--reuse-probe', type=Path)
    args = parser.parse_args()
    assert not args.output.exists(), 'Use a new output path for a new run'
    proof = json.loads(args.validation.read_text())
    sources = proof['source_sha256']
    assert set(args.units) <= sources.keys()

    def unchanged():
        assert all(sha(ROOT / name) == digest for name, digest in sources.items())

    unchanged()
    compiler = Path(shutil.which(args.cxx) or args.cxx).resolve()
    flags = ['-std=c++23', '-O1', '-Wall', '-Wextra', '-Werror', '-DDEBUG=1', '-I', str(ROOT),
             '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-g']
    environment = os.environ.copy()
    environment['ASAN_OPTIONS'] = 'detect_leaks=0'
    environment['UBSAN_OPTIONS'] = 'halt_on_error=1'
    args.work.mkdir(parents=True, exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='native-sanitize-', dir=args.work))
    report = dict(started_at=datetime.now(timezone.utc).isoformat(),
                  baseline_commit=proof['baseline_commit'], source_sha256=sources,
                  script_sha256=sha(Path(__file__)), compiler_version=subprocess.check_output([args.cxx, '--version'], text=True),
                  compiler_sha256=sha(compiler), flags=flags,
                  environment={name: environment[name] for name in ['ASAN_OPTIONS', 'UBSAN_OPTIONS']},
                  units=args.units, work=str(work), checks=[], completed=False,
                  flags_per_unit={'test/unit/fps_evaluation.cpp': ['-O2' if flag == '-O1' else flag for flag in flags] + ['-Wno-error=array-bounds']},
                  warning_policy='For the evaluation unit only, retain the baseline-identical unreachable Part[1] array-bounds diagnostic without treating it as a compile error; all runtime sanitizer checks remain enabled. StandardLayout kBase=0 reaches coefficients only for count=1, which returns before the merge loop.')
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def save():
        args.output.write_text(json.dumps(report, indent=2) + '\n')

    def run(command, label):
        log = work / (label + '.log')
        start = time.monotonic()
        with open('/tmp/cp-fps-tuning.lock', 'a') as lock, log.open('wb') as output:
            fcntl.flock(lock, fcntl.LOCK_SH)
            completed = subprocess.run(command, cwd=ROOT, env=environment, stdout=output,
                                       stderr=subprocess.STDOUT, timeout=300)
        row = dict(command=command, returncode=completed.returncode, log=str(log), log_sha256=sha(log),
                   seconds=time.monotonic() - start)
        report['checks'].append(row)
        save()
        assert completed.returncode == 0, (command, log.read_text()[-8000:])
        return row

    save()
    native = work / 'native.cpp'
    native.write_text('#include "components/ntt.hpp"\n#include <cstdio>\nint main() {\n'
                      '#if !defined(__linux__) || !defined(__x86_64__) || !defined(__GNUC__) || defined(__clang__)\n'
                      'return 1;\n#else\n'
                      'if (!fps_detail::NTT::vectorized()) return 2;\n'
                      'std::puts("Linux GCC native AVX2 path available"); return 0;\n#endif\n}\n')
    probe = work / 'native'
    if args.reuse_probe:
        previous = json.loads(args.reuse_probe.read_text())
        assert previous['source_sha256'] == sources and previous['flags'] == flags
        assert previous['compiler_sha256'] == report['compiler_sha256']
        reused = previous['checks'][:2]
        assert len(reused) == 2 and all(row['returncode'] == 0 for row in reused)
        assert reused[0]['command'][:-3] == [args.cxx, *flags]
        assert Path(reused[0]['command'][-3]).read_bytes() == native.read_bytes()
        assert all(sha(Path(row['log'])) == row['log_sha256'] for row in reused)
        report['checks'].extend(reused)
        report['reused_probe'] = dict(path=str(args.reuse_probe), sha256=sha(args.reuse_probe))
        save()
    else:
        run([args.cxx, *flags, str(native), '-o', str(probe)], 'native-compile')
        run([str(probe)], 'native-run')
    for name in args.units:
        binary = work / Path(name).stem
        actual_flags = report['flags_per_unit'].get(name, flags)
        row = run([args.cxx, *actual_flags, str(ROOT / name), '-o', str(binary)], Path(name).stem + '-compile')
        row['binary_sha256'] = sha(binary)
        row['source'] = name
        run([str(binary)], Path(name).stem + '-run')
        print('GCC native ASan/UBSan', name, 'passed', flush=True)
    unchanged()
    assert sha(compiler) == report['compiler_sha256']
    assert sha(Path(__file__)) == report['script_sha256']
    report.update(completed=True, sources_unchanged=True, completed_at=datetime.now(timezone.utc).isoformat())
    save()


if __name__ == '__main__':
    main()
