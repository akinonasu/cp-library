import argparse
import gzip
import hashlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import tarfile


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, default=Path(__file__).resolve().parents[5])
    parser.add_argument('--work', type=Path, required=True)
    args = parser.parse_args()
    package = Path(__file__).resolve().parent
    expected = json.loads((package / 'header_hashes.json').read_text())
    meta = json.loads((package / 'metadata.json').read_text())
    args.work.mkdir(parents=True, exist_ok=False)
    base = args.work / 'base'
    base.mkdir()
    archive = subprocess.check_output(['git', '-C', str(args.repo), 'archive', meta['baseline']])
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        tar.extractall(base, filter='data')
    trees = {'baseline': base}
    for name in expected['variants']:
        tree = args.work / name
        shutil.copytree(base, tree)
        subprocess.run(['git', 'apply', str(package / 'patches' / (name + '.patch'))], cwd=tree, check=True)
        trees[name] = tree
    audit = {}
    for name, tree in trees.items():
        actual = {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*.hpp'))}
        reference = expected['baseline'] if name == 'baseline' else expected['variants'][name]
        assert actual == reference, name
        audit[name] = dict(headers=len(actual), match=True)
    refs = args.work / 'references'
    for problem, row in meta['reference_sources'].items():
        compressed = package / row['compressed_path']
        assert sha(compressed) == row['compressed_sha256']
        out = refs / problem / (str(row['id']) + '.cpp')
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(gzip.decompress(compressed.read_bytes()))
        assert sha(out) == row['source_sha256']
    (args.work / 'reconstruction.json').write_text(json.dumps(audit, indent=2) + '\n')
    print(audit)


if __name__ == '__main__':
    main()
