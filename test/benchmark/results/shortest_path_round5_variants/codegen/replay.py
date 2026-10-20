import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess

ARCHIVE = Path(__file__).resolve().parent
ROOT = ARCHIVE.parents[4]
parser = argparse.ArgumentParser()
parser.add_argument('--destination', type=Path, default=ROOT / '.verify-helper/cache/shortest-path/round5/codegen-replay')
parser.add_argument('--audit-output', type=Path, default=ARCHIVE / 'reconstruction_audit.json')
args = parser.parse_args()
destination = args.destination.resolve()
manifest_path = ARCHIVE / 'source_manifest.json'
manifest = json.loads(manifest_path.read_text())

def sha(data):
    return hashlib.sha256(data).hexdigest()

checks = {}
for name, item in manifest['variants'].items():
    tree = destination / name
    for relative, digest in item['files'].items():
        target = tree / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        if relative in manifest['drivers']:
            data = (ARCHIVE / relative).read_bytes()
            assert sha(data) == manifest['drivers'][relative]
        else:
            data = subprocess.check_output(['git', 'show', item['revision'] + ':' + relative], cwd=ROOT).replace(b'\r\n', b'\n')
        target.write_bytes(data)
    if 'patch' in item:
        patch = ARCHIVE / item['patch']
        assert sha(patch.read_bytes()) == item['patch_sha256']
        subprocess.run(['patch', '--batch', '--forward', '-p1', '-d', str(tree), '-i', str(patch)], check=True, stdout=subprocess.DEVNULL)
    actual = {str(p.relative_to(tree)): sha(p.read_bytes()) for p in tree.rglob('*') if p.is_file() and p.suffix in ('.hpp', '.cpp')}
    assert actual == item['files'], name
    checks[name] = dict(revision=item['revision'], files=len(actual), all_source_hashes_match=True)

measurements = {}
for filename in ['screen_native.json', 'pointer_native.json', 'destination_native.json']:
    data = json.loads((ARCHIVE / filename).read_text())
    for name, files in data['source_hashes'].items():
        expected = manifest['variants'][name]['files']
        supplied = {path: digest for path, digest in files.items() if Path(path).suffix in ('.cpp', '.hpp')}
        assert supplied == expected, (filename, name)
    assert data['validation'] == 'all distances and warmup paths passed'
    assert all(len(rows) == data['trials'] for case in data['samples'].values() for rows in case['raw'].values())
    measurements[filename] = dict(cases=len(data['samples']), trials=data['trials'], frozen_source_hashes_match=True, all_trial_counts_match=True)

audit = dict(audited_at=datetime.now(timezone.utc).isoformat(), destination=str(destination), source_manifest_sha256=sha(manifest_path.read_bytes()), replay_sha256=sha(Path(__file__).read_bytes()), variants=checks, measurements=measurements)
args.audit_output.parent.mkdir(parents=True, exist_ok=True)
args.audit_output.write_text(json.dumps(audit, indent=2) + '\n')
print('All', len(checks), 'candidate source trees and', len(measurements), 'measurement archives match')
