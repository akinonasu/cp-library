import fcntl
import json
import os
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / '.verify-helper'))
from documentation import generate


if __name__ == '__main__':
    os.chdir(ROOT)
    headers = [path.relative_to(ROOT).as_posix()
               for folder in ['components', 'convolution', 'fps', 'data_structure', 'graph', 'math', 'string']
               for path in (ROOT / folder).rglob('*.hpp')]
    judges = [path.relative_to(ROOT).as_posix() for path in (ROOT / 'test').rglob('*.test.cpp')]
    affected = ['components/overwrite_array.hpp', 'components/directed_csr.hpp', 'graph/dijkstra.hpp']
    selection = ROOT / '.verify-helper/cache/shortest-path/round5/docs_selection.json'
    selection.parent.mkdir(parents=True, exist_ok=True)
    plan = dict(headers=affected, bundles=affected,
                state=dict(targets=dict(headers=headers, verification=judges)))
    selection.write_text(json.dumps(plan, indent=2) + '\n')
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        generate(plan)
        subprocess.run([sys.executable, '.verify-helper/check-docs.py', '--selection', str(selection)], check=True)
