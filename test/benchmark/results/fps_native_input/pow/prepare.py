from pathlib import Path
import argparse
import hashlib
import json
import shutil
import subprocess


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def verify_headers(root, expected):
    actual = {
        str(path.relative_to(root)): sha256(path)
        for path in root.rglob('*.hpp')
    }
    if actual != expected:
        changed = sorted(
            name for name in actual.keys() | expected.keys()
            if actual.get(name) != expected.get(name)
        )
        raise RuntimeError(f'header mismatch: {changed}')


parser = argparse.ArgumentParser()
parser.add_argument('repository', type=Path)
parser.add_argument('output', type=Path)
parser.add_argument(
    '--judge-dir', type=Path,
    default=Path('/tmp/library-checker-problems/polynomial/pow_of_formal_power_series'),
)
args = parser.parse_args()
package = Path(__file__).resolve().parent
repository = args.repository.resolve()
output = args.output.resolve()
judge = args.judge_dir.resolve()
output.mkdir(parents=True, exist_ok=False)
metadata = json.loads((package / 'metadata.json').read_text())
manifest = json.loads((package / 'manifest.json').read_text())
for name, expected in manifest.items():
    if name == 'reference_kernel.cpp':
        continue
    if sha256(package / name) != expected:
        raise RuntimeError(f'package hash mismatch: {name}')

baseline = output / 'baseline'
candidate = output / 'candidate'
baseline.mkdir()
archive = subprocess.Popen(
    ['git', '-C', str(repository), 'archive', '4765ceb'],
    stdout=subprocess.PIPE,
)
try:
    subprocess.run(['tar', '-xf', '-', '-C', str(baseline)], stdin=archive.stdout, check=True)
finally:
    archive.stdout.close()
if archive.wait():
    raise RuntimeError('git archive failed')
verify_headers(baseline, metadata['sources']['baseline_headers'])
shutil.copytree(baseline, candidate)
for patch in ['fps-blocked-native.patch', 'pow-native.patch']:
    with (package / patch).open('rb') as source:
        subprocess.run(['patch', '-p1'], cwd=candidate, stdin=source, check=True)
verify_headers(candidate, metadata['sources']['candidate_headers'])

for name in ['kernel.cpp', 'safety.cpp', 'ref_kernel_tail.cpp', 'build.py', 'check.py', 'bench.py']:
    shutil.copy2(package / name, output / name)
reference = output / 'reference409718.cpp'
shutil.copy2(package / reference.name, reference)
if sha256(reference) != metadata['sources']['reference_whole']['source_sha256']:
    raise RuntimeError('reference source hash mismatch')
source = reference.read_text()
source = source[:source.index('namespace {\n\nconstexpr std::uint32_t kP = poly::kModulus;')]
(output / 'reference_kernel.cpp').write_text(source + (package / 'ref_kernel_tail.cpp').read_text())
if sha256(output / 'reference_kernel.cpp') != manifest['reference_kernel.cpp']:
    raise RuntimeError('reference kernel hash mismatch')

for name in ['build', 'check', 'bench']:
    source = (package / f'{name}.py').read_text()
    source = source.replace("Path('/tmp/fps-pow-native-probe')", f'Path({str(output)!r})')
    source = source.replace(
        "Path('/tmp/fps-reference/pow_of_formal_power_series/409718.cpp')",
        f'Path({str(reference)!r})',
    )
    source = source.replace(
        "Path('/tmp/library-checker-problems/polynomial/pow_of_formal_power_series')",
        f'Path({str(judge)!r})',
    )
    (output / f'{name}_local.py').write_text(source)

checks = json.loads((package / 'checks.json').read_text())
inputs = [row for row in checks if row['name'].endswith('.in')]
available = all((judge / 'in' / row['name']).is_file() for row in inputs)
if available:
    for row in inputs:
        if sha256(judge / 'in' / row['name']) != row['input_sha256']:
            raise RuntimeError(f'official input hash mismatch: {row["name"]}')

verification = {
    'baseline': '4765ceb',
    'baseline_headers_matched': len(metadata['sources']['baseline_headers']),
    'candidate_headers_matched': len(metadata['sources']['candidate_headers']),
    'candidate_changed_headers': [
        name for name, expected in metadata['sources']['candidate_headers'].items()
        if metadata['sources']['baseline_headers'].get(name) != expected
    ],
    'reference_whole_source_sha256': sha256(reference),
    'reference_kernel_source_sha256': sha256(output / 'reference_kernel.cpp'),
    'official_input_hashes_matched': len(inputs) if available else 0,
}
(output / 'reconstruction.json').write_text(json.dumps(verification, indent=2) + '\n')
print(json.dumps(verification, indent=2))
