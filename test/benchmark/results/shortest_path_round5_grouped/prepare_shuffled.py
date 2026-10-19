from pathlib import Path
import fcntl,random,hashlib,json,os,subprocess,sys
HERE=Path(__file__).resolve().parent
ROOT=Path(subprocess.check_output(['git','rev-parse','--show-toplevel'],text=True).strip())
sys.path.insert(0,str(ROOT/'test/benchmark'))
from shortest_path import synthetic_cases
os.sched_setaffinity(0,{2})
with open('/tmp/cp-wm-opt.lock','w') as lock:
    fcntl.flock(lock,fcntl.LOCK_EX)
    source=ROOT/'.verify-helper/cache/shortest-path/round4/inputs/synthetic_dense.in'
    if not source.exists():
        synthetic_cases(HERE/'inputs')
        source=HERE/'inputs/synthetic_dense.in'
    lines=source.read_bytes().splitlines(True)
    header=lines[0];edges=lines[1:]
    random.Random(2026101905).shuffle(edges)
    directory=HERE/'inputs';directory.mkdir(exist_ok=True)
    target=directory/'synthetic_dense_shuffled.in'
    target.write_bytes(header+b''.join(edges))
    (HERE/'shuffled_input.json').write_text(json.dumps(dict(source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),target_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),seed=2026101905,method='Python random.Random(seed).shuffle(edge lines), keeping header and edges unchanged'),indent=2)+'\n')
