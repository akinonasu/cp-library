from concurrent.futures import ThreadPoolExecutor
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

ROOT = Path('/workspace/cp-library')
OUT = Path('/workspace/cp-fps-scratch-round3/upstream_f4aa')
RAW = ROOT / 'test/benchmark/results/fps_round3_all_speed.json'
WORK = Path('/workspace/cp-fps-scratch-round3/fps-round3-all-speed')
CURRENT = WORK / 'current'
SAVED = WORK / 'current_original_f3ac5ae'
NEW = WORK / 'current_upstream_f4aa'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def inventory(tree):
    return {str(p.relative_to(tree)): sha(p) for p in sorted(tree.rglob('*')) if p.is_file()}


def save(value):
    (OUT / 'integration.json').write_text(json.dumps(value, indent=2) + '\n')


def main():
    raw = json.loads(RAW.read_text())
    validation_path = ROOT / 'test/benchmark/results/shortest_path_io_validation.json'
    validation = json.loads(validation_path.read_text())
    assert all(sha(ROOT / name) == digest for name, digest in validation['sources'].items())
    assert len(validation['checks']) == 8 and all(row['status'] == 'passed' for row in validation['checks'])
    original = {str(RAW): sha(RAW)}
    for row in raw['judge']:
        for variant in ['before', 'current', 'reference']:
            record = raw['sources'][row['problem'] + '_' + variant]
            binary = Path(record['command'][-1])
            original[str(binary)] = sha(binary)
            assert original[str(binary)] == record['binary_sha256']
            output = Path(str(binary) + '.out')
            original[str(output)] = sha(output)
            assert original[str(output)] == row['variants'][variant]['output_sha256']
    prior_tree = inventory(CURRENT)
    old_fps = (CURRENT / 'components/fps_io.hpp').read_text()
    new_fps = (ROOT / 'components/fps_io.hpp').read_text()
    bulk = (ROOT / 'components/fast_io_bulk.hpp').read_text()
    start, stop = old_fps.index('namespace fast_io_detail {'), old_fps.index('namespace fps_io_fields {')
    old_body = old_fps[start:stop]
    new_body = bulk[bulk.index('namespace fast_io_detail {'):bulk.index('#pragma GCC pop_options')]
    recovered = new_body.replace('\t\tbool chunks = true;\n', '').replace('while (chunks && count - done', 'while (count - done')
    recovered = recovered.replace('if (!parsed) {\n\t\t\t\t\tchunks = false;\n\t\t\t\t\tbreak;\n\t\t\t\t}', 'if (!parsed) break;')
    assert old_body.split() == recovered.split()
    assert (old_fps[:start] + old_fps[stop:]).split() == new_fps.replace('"fast_io_bulk.hpp"', '"fast_io.hpp"').split()
    result = dict(started=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  head=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
                  original_raw_sha256=sha(RAW), original_artifacts=original,
                  original_tree=prior_tree, overlay={name: sha(ROOT / name) for name in ['components/fps_io.hpp', 'components/fast_io_bulk.hpp']},
                  validation_evidence=dict(path=str(validation_path), sha256=sha(validation_path), checks=8, all_sources_match=True, all_passed=True),
                  extraction_review=dict(bulk_body_equal_except_local_chunks=True, fps_caller_body_unchanged=True,
                      safety='Rejected chunk commits neither p nor emit; local flag only selects existing bounded two-token/scalar fallback thereafter.'),
                  builds={}, status='prepared')
    save(result)
    assert not SAVED.exists() and not NEW.exists()
    with open('/tmp/cp-fps-tuning.lock', 'a') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        CURRENT.rename(SAVED)
        try:
            shutil.copytree(SAVED, CURRENT)
            for name in result['overlay']:
                shutil.copyfile(ROOT / name, CURRENT / name)
            candidate = inventory(CURRENT)
            delta = {name: [prior_tree.get(name), digest] for name, digest in candidate.items() if prior_tree.get(name) != digest}
            assert set(delta) == set(result['overlay'])
            result['candidate_tree'] = candidate
            result['source_delta'] = delta
            result['status'] = 'building'
            save(result)

            def build(row):
                problem = row['problem']
                record = raw['sources'][problem + '_current']
                command = record['command'].copy()
                binary = Path(command[-1] + '_upstream_f4aa')
                command[-1] = str(binary)
                started = time.monotonic()
                completed = subprocess.run(command, cwd=ROOT, env=os.environ | {'TMPDIR': str(OUT)}, capture_output=True, text=True)
                data = dict(command=command, source_sha256=sha(Path(command[-3])), returncode=completed.returncode,
                            stdout=completed.stdout, stderr=completed.stderr, seconds=time.monotonic() - started,
                            binary_sha256=sha(binary) if completed.returncode == 0 else None,
                            original_binary_sha256=record['binary_sha256'])
                data['byte_identical'] = completed.returncode == 0 and data['binary_sha256'] == record['binary_sha256']
                return problem, data

            with ThreadPoolExecutor(max_workers=3) as pool:
                for problem, data in pool.map(build, raw['judge']):
                    result['builds'][problem] = data
                    save(result)
                    print(problem, data['returncode'], data['byte_identical'], flush=True)
        finally:
            if CURRENT.exists():
                CURRENT.rename(NEW)
            SAVED.rename(CURRENT)
    result['restored_original_tree'] = inventory(CURRENT) == prior_tree
    result['original_artifacts_preserved'] = all(sha(Path(path)) == digest for path, digest in original.items())
    result['live_overlay_unchanged'] = all(sha(ROOT / name) == digest for name, digest in result['overlay'].items())
    result['all_compiled'] = len(result['builds']) == 16 and all(row['returncode'] == 0 for row in result['builds'].values())
    result['affected'] = [problem for problem, row in result['builds'].items() if not row['byte_identical']]
    result['status'] = 'byte-identical' if result['all_compiled'] and not result['affected'] else 'changed'
    assert result['restored_original_tree'] and result['original_artifacts_preserved'] and result['live_overlay_unchanged']
    result['finished'] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    save(result)
    print(result['status'], result['affected'], flush=True)


if __name__ == '__main__':
    main()
