import sys
from pathlib import Path
import subprocess
import re
import shutil
import json
ROOT = Path(__file__).resolve().parents[5]
sys.path.insert(0, str(ROOT / 'test/benchmark'))
from shortest_path_round3 import freeze_reference
WORK = Path(__file__).resolve().parent
variants = {'old': 'f4aa743', 'current': None}
metadata = {}
for name, revision in variants.items():
    directory = WORK / name
    if directory.exists(): shutil.rmtree(directory)
    entry, hashes, manifest = freeze_reference(directory, ROOT / 'test/dijkstra_path.test.cpp', revision)
    metadata[name] = dict(entry=str(entry), hashes=hashes, manifest=manifest)
(WORK / 'metadata.json').write_text(json.dumps(metadata, indent=2))
print(ROOT)
print(json.dumps(metadata, indent=2))
