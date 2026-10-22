import argparse, fcntl, hashlib, json, re, subprocess, tempfile, time
from pathlib import Path
parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default='/workspace/cp-binomial-study/bin/clang++')
parser.add_argument('--output', type=Path)
args = parser.parse_args()
artifacts = Path(__file__).resolve().parent
root = next(p for p in artifacts.parents if (p / 'fps/fps.hpp').exists())
seen = {}
def record(path):
    path = path.resolve()
    if path in seen:
        return
    seen[path] = hashlib.sha256(path.read_bytes()).hexdigest()
    for name in re.findall(r'^\s*#include\s+"([^"]+)"', path.read_text(), re.M):
        dep = path.parent / name
        if not dep.exists():
            dep = root / name
        if dep.exists():
            record(dep)
record(artifacts / 'log_precision.cpp')
flags = ['-std=c++20', '-O2', '-I', str(root)]
result = {'purpose': 'Correctness at the preserved log precision endpoint, not a performance benchmark.', 'date_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()), 'compiler': subprocess.check_output([args.compiler, '--version'], text=True).splitlines()[0], 'compiler_path': args.compiler, 'flags': flags, 'header_sha256': {str(p.relative_to(root)): h for p, h in sorted(seen.items())}, 'input': 'f(x) = 1 + x + ... + x^64, degree = 2^24', 'closed_form': 'For i>0, i*[x^i]log(f) = 1 if 65 does not divide i, and -64 otherwise. This follows from f=(1-x^65)/(1-x).', 'assertions': 'enabled', 'platform': 'Linux x86_64 executing the Clang fallback; this does not claim execution on physical Mac hardware.'}
with tempfile.TemporaryDirectory(prefix='fps-clang-log-') as temp:
    executable = Path(temp) / 'log_precision'
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        command = [args.compiler, *flags, str(artifacts / 'log_precision.cpp'), '-o', str(executable)]
        compiled = subprocess.run(command, text=True, capture_output=True)
        result['compile_command'] = command
        result['compile_exit_code'] = compiled.returncode
        result['compile_stderr'] = compiled.stderr
        if compiled.returncode:
            raise RuntimeError(compiled.stderr)
        result['binary_sha256'] = hashlib.sha256(executable.read_bytes()).hexdigest()
        start = time.monotonic()
        run = subprocess.run([str(executable)], text=True, capture_output=True)
        result['correctness_wall_seconds'] = time.monotonic() - start
        result['run_exit_code'] = run.returncode
        result['stdout'] = run.stdout
        result['resource_output'] = run.stderr
        result['unchanged_sources'] = all(hashlib.sha256(p.read_bytes()).hexdigest() == h for p, h in seen.items())
        result['pass'] = run.returncode == 0 and result['unchanged_sources']
        if run.returncode == 0:
            result['checked'] = json.loads(run.stdout)
output = args.output or artifacts / 'log_precision.json'
output.write_text(json.dumps(result, indent=2) + '\n')
print(json.dumps({k: result[k] for k in ['run_exit_code', 'pass', 'stdout', 'unchanged_sources']}, indent=2))
if not result['pass']:
    raise SystemExit(1)
