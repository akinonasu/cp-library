import argparse
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    parser.add_argument('--experiments', nargs='+', default=['tiles9', 'scheduling9', 'leaf32_9', 'leaf32asm_9', 'karatsuba4_9'])
    args = parser.parse_args()
    archive = Path(__file__).resolve().parent
    manifest = json.loads((archive / 'manifest.json').read_text())
    for name, record in manifest['files'].items():
        assert sha(archive / name) == record['sha256'], name
    args.work.mkdir(parents=True)
    raw = {name: json.loads((archive / (name + '.json')).read_text()) for name in args.experiments}
    variants = sorted(set().union(*(set(x['sources']) - {'reference'} for x in raw.values())))
    data = subprocess.check_output(['git', 'archive', manifest['baseline'], 'fps', 'components', 'math', 'convolution', 'test/convolution.test.cpp', 'test/fps_io.hpp'], cwd=args.repo)
    base = args.work / 'base'
    base.mkdir()
    with tarfile.open(fileobj=io.BytesIO(data)) as source:
        source.extractall(base, filter='data')
    assert sha(base / 'components/fps_convolution.hpp') == manifest['base_header_sha256']
    for variant in variants:
        tree = args.work / variant
        if variant != 'base':
            shutil.copytree(base, tree)
        shutil.copyfile(archive / 'sources' / (variant + '.hpp'), tree / 'components/fps_convolution.hpp')
    spec = importlib.util.spec_from_file_location('kernel_probe', archive / 'probe.py')
    probe = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(probe)
    probe.ROOT, probe.WORK = args.repo, args.work
    probe.REFERENCE, probe.OFFICIAL = archive / 'reference409465.cpp', args.official
    for name, result in raw.items():
        sys.argv = ['probe.py', '--trials', str(result['trials']), '--variants', *result['sources'],
                    '--cases', *(x['case'] for x in result['rows']), '--output', str(args.work / (name + '.json'))]
        probe.main()


if __name__ == '__main__':
    main()
