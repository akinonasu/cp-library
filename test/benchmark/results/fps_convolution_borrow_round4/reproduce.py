import argparse
import gzip
import hashlib
import json
from pathlib import Path
import subprocess
import tarfile


HERE = Path(__file__).resolve().parent


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--work', type=Path, required=True)
    parser.add_argument('--official', type=Path, default=Path('/tmp/library-checker-problems/convolution/convolution_mod'))
    parser.add_argument('--prepare-only', action='store_true')
    args = parser.parse_args()
    args.work = args.work.resolve()
    args.work.mkdir(parents=True, exist_ok=True)
    audit = json.loads((HERE / 'audit.json').read_text())
    screen = json.loads((HERE / 'screen_result.json').read_text())
    confirm = json.loads((HERE / 'custom_confirm.json').read_text())
    assert sha(HERE / 'baseline.tar.gz') == audit['baseline_archive_sha256']
    trees = {}
    for name, patch in [('before', None), ('borrowed', 'fallback_borrow.patch'), ('split', 'fallback_split.patch')]:
        tree = args.work / name
        assert not tree.exists(), 'Use a new work directory'
        with tarfile.open(HERE / 'baseline.tar.gz') as archive:
            archive.extractall(tree, filter='data')
        if patch:
            subprocess.run(['git', 'apply', str(HERE / patch)], cwd=tree, check=True)
        expected = screen['sources']['borrowed' if name == 'borrowed' else 'before']
        for path, digest in expected.items():
            if name == 'split' and path == 'components/fps_convolution.hpp':
                digest = audit['split_source_sha256']
            assert sha(tree / path) == digest, (name, path)
        trees[name] = tree
    reference = args.work / '409465.cpp'
    reference.write_bytes(gzip.decompress((HERE / '409465.cpp.gz').read_bytes()))
    assert sha(reference) == audit['reference_source_sha256']
    for name in ['custom_bench.py', 'custom_confirm.py']:
        record = json.loads((HERE / ('custom_first.json' if name == 'custom_bench.py' else 'custom_confirm.json')).read_text())
        assert sha(HERE / name) == record['script_sha256']
    assert sha(HERE / 'screen_driver.py') == screen['harness_sha256']
    print('Prepared exact 109-header baseline, borrowed and split variants; original reference and harness hashes match.', flush=True)
    if args.prepare_only:
        return
    import fcntl
    binary_dir = args.work / 'fallback_first'
    binary_dir.mkdir()
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        for name in confirm['binary_sha256']:
            source = reference if name == 'reference' else trees[name] / 'test/convolution.test.cpp'
            command = ['g++', *screen['flags']]
            if name != 'reference':
                command += ['-I' + str(trees[name])]
            command += [str(source), '-o', str(binary_dir / name)]
            subprocess.run(command, check=True)
    source = (HERE / 'custom_confirm.py').read_text()
    old = "W=Path('/workspace/cp-fps-round4/root')"
    assert source.count(old) == 1
    source = source.replace(old, 'W=Path(' + repr(str(args.work)) + ')')
    old = "Path('/tmp/library-checker-problems/convolution/convolution_mod/checker')"
    assert source.count(old) == 1
    source = source.replace(old, 'Path(' + repr(str(args.official.resolve() / 'checker')) + ')')
    old = "Path('/tmp/library-checker-problems/convolution/convolution_mod')"
    assert source.count(old) == 1
    source = source.replace(old, 'Path(' + repr(str(args.official.resolve())) + ')')
    runner = args.work / 'replay_confirm.py'
    runner.write_text(source)
    subprocess.run(['python3', str(runner)], check=True)


if __name__ == '__main__':
    main()
