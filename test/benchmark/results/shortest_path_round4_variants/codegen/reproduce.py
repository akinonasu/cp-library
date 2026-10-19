import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, default=HERE.parents[4])
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--variant', action='append')
    args = parser.parse_args()
    repo = args.repo.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((HERE / 'manifest.json').read_text())['variants']
    selected = args.variant or list(manifest)
    for name in selected:
        settings = manifest[name]
        destination = output / name
        destination.mkdir()
        with tempfile.TemporaryDirectory(prefix='cp-sp-replay-', dir=output) as temporary:
            canonical = Path(temporary)
            for key in settings['canonical_to_flat']:
                data = subprocess.check_output(['git', 'show', settings['base_revision'] + ':' + key], cwd=repo)
                target = canonical / key
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data.replace(b'\r\n', b'\n'))
            if settings['patch']:
                subprocess.run(['patch', '-p1', '--batch', '--forward', '-i', str(HERE / settings['patch'])], cwd=canonical, check=True, stdout=subprocess.DEVNULL)
            for key, flat in settings['canonical_to_flat'].items():
                data = (canonical / key).read_text().replace('\r\n', '\n')
                assert sha(data.encode()) == settings['canonical_sha256'][key], (name, key)

                def include(match):
                    target = os.path.normpath(str(Path(key).parent / match[2])).replace('\\', '/')
                    if target not in settings['canonical_to_flat']:
                        return match[0]
                    return match[1] + settings['canonical_to_flat'][target] + match[3]

                data = re.sub(r'(^\s*#include\s+")([^"\r\n]+)(")', include, data, flags=re.M)
                assert sha(data.encode()) == settings['compiled_sha256'][flat], (name, flat)
                (destination / flat).write_text(data)
    for script in ('measure.py', 'boundary.py'):
        shutil.copyfile(HERE / 'scripts' / script, output / script)
    (output / 'replay.json').write_text(json.dumps(dict(variants=selected, source_hashes_verified=True), indent=2) + '\n')
    print('Verified', len(selected), 'variants')


if __name__ == '__main__':
    main()
