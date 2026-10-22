import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
parser = argparse.ArgumentParser()
parser.add_argument('--bundle', default=shutil.which('oj-bundle'), required=shutil.which('oj-bundle') is None)
parser.add_argument('--cxx', nargs='+', default=['g++', 'clang++'])
parser.add_argument('--output', type=Path, default=ROOT / 'test/benchmark/results/fps_light/bundle.json')
args = parser.parse_args()
report = {'started_at': datetime.now(timezone.utc).isoformat(), 'method': 'oj-bundle -I ROOT source.cpp plus reference URL, original formatting and all platform branches, no compacting', 'limit_bytes': 65536, 'bundle': {'path': args.bundle, 'sha256': hashlib.sha256(Path(args.bundle).resolve().read_bytes()).hexdigest()}, 'rows': [], 'compilers': {}, 'checks': [], 'completed': False}
args.output.parent.mkdir(parents=True, exist_ok=True)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def save():
    args.output.write_text(json.dumps(report, indent=2) + '\n')

with open('/tmp/cp-fps-tuning.lock', 'w') as lock, open('/tmp/cp-wm-opt.lock', 'w') as wm_lock, tempfile.TemporaryDirectory(prefix='fps-light-bundle-') as work:
    fcntl.flock(lock, fcntl.LOCK_SH)
    fcntl.flock(wm_lock, fcntl.LOCK_SH)
    work = Path(work)
    inputs = []
    for header in sorted((ROOT / 'fps').glob('*light*.hpp')):
        for io in [False, True]:
            source = work / (header.stem + ('_io' if io else '') + '.cpp')
            text = '#include "' + header.relative_to(ROOT).as_posix() + '"\n'
            if io:
                text += '#include "components/fast_io.hpp"\n'
            source.write_text(text + 'int main() {}\n')
            inputs.append((source, header.relative_to(ROOT).as_posix(), io))
    inputs += [(path, path.relative_to(ROOT).as_posix(), None) for path in sorted((ROOT / 'test').glob('fps_light_*.test.cpp'))]
    for source, name, io in inputs:
        command = [args.bundle, '-I', str(ROOT), str(source)]
        result = subprocess.run(command, cwd=ROOT, check=True, capture_output=True)
        data = result.stdout
        reference = ('//https://qandori.github.io/cp-library/lightweight/' + name + '.html\n').encode()
        copy_bytes = len(reference + data)
        assert copy_bytes < 65536, (name, copy_bytes)
        if io is False:
            assert data.isascii(), name
        output = work / (source.stem + '.bundled.cpp')
        output.write_bytes(data)
        report['rows'].append({'source': name, 'with_fast_io': io, 'source_sha256': sha(source), 'bytes': len(data), 'copy_bytes': copy_bytes, 'sha256': sha(output), 'remaining_bytes': 65536 - copy_bytes, 'command': command})
        if name in ['test/fps_light_exp.test.cpp', 'test/fps_light_linear_recurrence.test.cpp']:
            for cxx in args.cxx:
                report['compilers'][cxx] = subprocess.check_output([cxx, '--version'], text=True)
                command = [cxx, '-std=c++23', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Werror', str(output), '-o', str(work / 'driver')]
                subprocess.run(command, check=True)
                report['checks'].append({'source': name, 'compiler': cxx, 'command': command, 'status': 'passed'})
        print(name, io, copy_bytes, flush=True)
        save()
report['completed'] = True
report['completed_at'] = datetime.now(timezone.utc).isoformat()
save()
