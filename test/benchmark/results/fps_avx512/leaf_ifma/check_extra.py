from pathlib import Path
import fcntl, hashlib, json, subprocess

work = Path(__file__).resolve().parent
flags = ['-std=c++23', '-O2', '-DEVAL', '-DONLINE_JUDGE', '-ftrivial-auto-var-init=zero', '-march=haswell']
def sources():
    paths = [*work.glob('*.cpp'), work / 'leaf_ifma.hpp']
    paths += [p for tree in ['base', 'base32', 'instrumented'] for p in (work / tree).rglob('*.hpp')]
    return {str(p.relative_to(work)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}
before = sources()
rows = {}
with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
    fcntl.flock(lock, fcntl.LOCK_SH)
    for name, tree in [('range', 'instrumented'), ('alignment_check', 'base32')]:
        command = ['g++', *flags, '-I', str(work / tree), str(work / (name + '.cpp')), '-o', str(work / name)]
        subprocess.run(command, check=True)
        run = subprocess.run([str(work / name)], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True)
        rows[name] = {'build': command, 'run': [str(work / name)], 'output': run.stdout,
                      'binary_sha256': hashlib.sha256((work / name).read_bytes()).hexdigest()}
        print(run.stdout, end='', flush=True)
after = sources()
assert before == after, 'source changed during checks'
(work / 'extra-checks.json').write_text(json.dumps({'flags': flags, 'checks': rows, 'sources_before': before, 'sources_after': after, 'sources_unchanged': True}, indent=2) + '\n')
