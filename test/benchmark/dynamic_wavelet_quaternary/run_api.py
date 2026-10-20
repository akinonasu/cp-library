#!/usr/bin/env python3
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import random
import shutil
import statistics
import subprocess
import time

ROOT = Path('/workspace/cp-library')
WORK = Path('/workspace/cp-dwm-study/final-api-controls')
BASE = '2559803022319333756173295947577d6f5f9b2e'
FLAGS = ['-std=gnu++23', '-O2', '-march=native', '-ftrivial-auto-var-init=zero',
         '-UNDEBUG', '-fno-lto', '-Wall', '-Wextra', '-Werror']
HEADERS = ['data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp', 'components/dynamic_bit_vector.hpp',
           'components/dynamic_quad_vector.hpp', 'components/dynamic_quad_wavelet.hpp']
EXPECTED = {'components/dynamic_quad_vector.hpp': '36a86dd0725c137075fa6900301f6f7b9a5a3b1c7ac9cf55318c28284b899a81',
            'components/dynamic_quad_wavelet.hpp': 'e8201e59e4afbb6845c038f36682248d55740cb9bbe8bf0d219486a254f999ea',
            'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp': '2db1f1f83a1ab0f879bd85b8bc4220e0c84f0b5a4dbb2756870e5b98fcb0b5ef'}
DOMAINS = [('i64', 'std::int64_t', 64), ('u64', 'std::uint64_t', 64),
           ('i31', 'std::int32_t', 31), ('u7', 'std::uint32_t', 7)]
KINDS = ['access', 'updates', 'set-noop', 'insert', 'erase', 'count', 'rank', 'select',
         'kth', 'freq', 'range', 'prev', 'next']
SEED = 20261020


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def emit(value):
    print(json.dumps(value), flush=True)


def run(command, **kwargs):
    result = subprocess.run(list(map(str, command)), capture_output=True, text=True, check=True, timeout=120, **kwargs)
    return result.stdout.strip()


def main():
    global ROOT, WORK, BASE, EXPECTED
    parser = argparse.ArgumentParser(description='Frozen baseline/final broad DynamicWaveletMatrix API controls.')
    parser.add_argument('--repo', type=Path, default=ROOT)
    parser.add_argument('--work', type=Path, required=True, help='Fresh private result directory; existing results are never replaced.')
    parser.add_argument('--baseline', default=BASE)
    parser.add_argument('--header-hashes', type=Path, required=True, help='JSON object mapping the final public/quad header paths to their expected SHA256.')
    args = parser.parse_args()
    ROOT, WORK, BASE = args.repo.resolve(), args.work.resolve(), args.baseline
    EXPECTED = json.loads(args.header_hashes.read_text())
    required = {'components/dynamic_quad_vector.hpp', 'components/dynamic_quad_wavelet.hpp', 'data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp'}
    if set(EXPECTED) != required:
        parser.error('--header-hashes must contain exactly the public matrix and both quad headers')
    if WORK == ROOT or ROOT in WORK.parents:
        parser.error('--work must be outside the production repository')
    if any((WORK / name).exists() for name in ['manifest.json', 'results.json', 'preflight.json']):
        parser.error('--work already contains measurement records; choose a fresh directory')
    WORK.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(Path(__file__).resolve(), WORK / 'measurement_run.py')
    headers = {}
    for arm in ['base', 'current']:
        headers[arm] = {}
        for relative in HEADERS:
            target = WORK / arm / relative
            if arm == 'base':
                result = subprocess.run(['git', '-C', str(ROOT), 'show', BASE + ':' + relative], capture_output=True)
                if result.returncode:
                    assert relative in ['components/dynamic_quad_vector.hpp', 'components/dynamic_quad_wavelet.hpp']
                    continue
                data = result.stdout
            else:
                data = (ROOT / relative).read_bytes()
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            headers[arm][relative] = sha(target)
            if arm == 'current' and relative in EXPECTED:
                assert headers[arm][relative] == EXPECTED[relative], relative
    source = (ROOT / 'test/benchmark/dwm_avx_final/bench.cpp').read_text()
    source = source.replace('using WM=DynamicWaveletMatrix<T,BITS,DynamicBitVector<BLOCK,BRANCH>>;', 'using WM=DynamicWaveletMatrix<T,BITS>;')
    assert source.count('if(kind=="access")t=6;') == 1
    source = source.replace('if(kind=="access")t=6;', 'if(kind=="set-noop")t=2;else if(kind=="access")t=6;')
    needle='  if(t==7)x=a.empty()?0:a[rng()%a.size()];'
    assert source.count(needle) == 1
    extra='''  if(kind=="set-noop")x=a[p];
  if(kind=="count"&&!(i&1)&&l<r)x=a[l+rng()%(r-l)];
  if(kind=="rank"&&!(i&1)&&r)x=a[rng()%r];
'''
    source = source.replace(needle, extra + needle)
    source = source.replace(';if(kind==', ';\n  if(kind==')
    source = source.replace(';return decode_key(x);', ';\n  return decode_key(x);')
    (WORK / 'bench.cpp').write_text(source)
    for name in ['escape.cpp', 'vector_oracle.cpp']:
        shutil.copyfile(ROOT / 'test/benchmark/dwm_avx_final' / name, WORK / name)
    manifest = {'base_commit': BASE, 'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                'headers': headers, 'flags': FLAGS, 'seed': SEED, 'cpu': 2,
                'domains': DOMAINS, 'kinds': KINDS, 'small_n_q': [1100, 1400], 'timed_n_q': [100000, 20000],
                'repeats': 3, 'compiler': run(['g++', '--version']),
                'compiler_sha256': sha(Path(shutil.which('g++')).resolve()),
                'lscpu': run(['lscpu']), 'uname': list(os.uname()),
                'sources': {name: sha(WORK / name) for name in ['measurement_run.py', 'bench.cpp', 'escape.cpp', 'vector_oracle.cpp']},
                'commands': [], 'binaries': {}}
    def compile_one(command):
        manifest['commands'].append(list(map(str, command)))
        result=subprocess.run(list(map(str,command)),capture_output=True,text=True)
        with (WORK/'compile.log').open('a') as stream:stream.write(result.stdout+result.stderr)
        result.check_returncode()
    oracle = WORK / 'vector_oracle'
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_SH)
        compile_one(['g++', *FLAGS, WORK / 'vector_oracle.cpp', '-o', oracle])
        compile_one(['g++', *FLAGS, '-c', WORK / 'escape.cpp', '-o', WORK / 'escape.o'])
        for name, value_type, bits in DOMAINS:
            for arm in ['base', 'current']:
                binary = WORK / (arm + '-' + name)
                dependency = binary.with_suffix('.d')
                compile_one(['g++', *FLAGS, '-DVALUE_TYPE=' + value_type, '-DBITS=' + str(bits),
                             '-I' + str(WORK / arm), '-MMD', '-MF', dependency,
                             WORK / 'bench.cpp', WORK / 'escape.o', '-o', binary])
                deps = dependency.read_text()
                for relative in headers[arm]:
                    # Relative includes can retain '..' spelling; verify resolved depfile paths.
                    words=deps.replace('\\\n',' ').split(':',1)[1].split()
                    resolved={Path(word).resolve() for word in words}
                    assert (WORK / arm / relative).resolve() in resolved, (arm,relative)
                manifest['binaries'][arm + '-' + name] = sha(binary)
                emit({'compiled': arm + '-' + name})
        (WORK / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
        preflight = []
        small = WORK / 'small'
        small.mkdir(exist_ok=True)
        for domain, _, _ in DOMAINS:
            for kind in KINDS:
                truth = None
                for arm in ['base', 'current']:
                    dump = small / (domain + '-' + kind + '-' + arm)
                    command = [WORK / (arm + '-' + domain), kind, '1100', '1400', str(SEED), 'uniform', 'random', '1', 'warm', dump, 'bulk']
                    answer = json.loads(run(command))
                    input_bytes = Path(str(dump) + '.input').read_bytes()
                    output_bytes = dump.read_bytes()
                    checked = json.loads(run([oracle, dump, kind]))
                    observed = (input_bytes, output_bytes, answer['hash'], answer['finalhash'])
                    if truth is not None:
                        assert observed == truth, ('small', domain, kind, arm)
                    truth = observed
                    preflight.append({'domain': domain, 'kind': kind, 'arm': arm,
                                      'command': list(map(str, command)), 'oracle': checked,
                                      'input_sha256': hashlib.sha256(input_bytes).hexdigest(),
                                      'full_output_final_sha256': hashlib.sha256(output_bytes).hexdigest()})
                emit({'preflight': domain + '-' + kind, 'status': 'pass'})
        (WORK / 'preflight.json').write_text(json.dumps(preflight, indent=2) + '\n')
    for relative, expected in headers['current'].items():
        assert sha(ROOT / relative) == expected, ('production source changed before timings', relative)
    result = {'manifest': manifest, 'small_oracle_preflights': len(preflight), 'cases': []}
    order = [(domain, kind) for domain, _, _ in DOMAINS for kind in KINDS]
    rng = random.Random(SEED)
    rng.shuffle(order)
    big = WORK / 'big'
    big.mkdir(exist_ok=True)
    env = dict(os.environ)
    for key in ['DWM_EXPECTED_HASH','DWM_EXPECTED_FINAL_HASH','DWM_VALIDATE_REPLAYS']:
        env.pop(key, None)
    with open('/tmp/cp-fps-tuning.lock','w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for domain, kind in order:
            case = {'domain': domain, 'kind': kind, 'samples': {'base': [], 'current': []}}
            truth = None
            for trial in range(3):
                arms = ['base','current']
                rng.shuffle(arms)
                for arm in arms:
                    dump=big/(domain+'-'+kind+'-'+arm+'-'+str(trial))
                    command = ['taskset','-c','2',WORK/(arm+'-'+domain),kind,'100000','20000',str(SEED),'uniform','random','1','warm',dump,'bulk']
                    answer=json.loads(run(command,env=env))
                    input_path=Path(str(dump)+'.input')
                    hashes=(sha(input_path),sha(dump),answer['hash'],answer['finalhash'])
                    if truth is not None:assert hashes==truth,('big differential',domain,kind,arm,trial)
                    truth=hashes
                    case['samples'][arm].append({'trial':trial,'command':list(map(str,command)),**answer})
                    # Keep one full input/output-final-state specimen per case, remove repeat copies.
                    if trial==0 and arm=='base':
                        input_path.rename(big/(domain+'-'+kind+'.input'))
                        dump.rename(big/(domain+'-'+kind+'.output-final'))
                    else:input_path.unlink();dump.unlink()
            case['input_sha256'],case['full_output_final_sha256'],case['output_digest'],case['final_digest']=truth
            case['summary']={}
            for metric in ['cpu','wall','buildcpu']:
                a=[s[metric] for s in case['samples']['base']]
                b=[s[metric] for s in case['samples']['current']]
                case['summary'][metric]={'base_median':statistics.median(a),'current_median':statistics.median(b),
                                         'current_over_base':statistics.median(b)/statistics.median(a),
                                         'base_range':[min(a),max(a)],'current_range':[min(b),max(b)]}
            cpu=case['summary']['cpu']
            case['material_regression']=cpu['current_over_base']>1.15
            case['below_50ms']=min(s['cpu'] for arm in case['samples'].values() for s in arm)<0.05
            result['cases'].append(case)
            (WORK/'results.json').write_text(json.dumps(result,indent=2)+'\n')
            emit({'case':[domain,kind],'cpu':cpu,'below_50ms':case['below_50ms'],'material_regression':case['material_regression']})
            if case['material_regression']:
                with (WORK/'regressions.jsonl').open('a') as stream:stream.write(json.dumps({'domain':domain,'kind':kind,'cpu':cpu,'below_50ms':case['below_50ms']})+'\n')
    for relative, expected in headers['current'].items():
        assert sha(ROOT/relative)==expected,('production source changed during timings',relative)
    result['complete']=True
    result['interpretation']='Three shuffled paired samples, descriptive API control. Small oracle validates every output and final-state word. Large cases are full-output/final-state differential checks, not a separate large independent oracle. Timing is operation-only; build/owner warming/hash/extraction/destruction are outside. No-op update values equal the original unchanged array. Count/rank choose existing values on half of queries. Every row retained; <50ms rows and >15% descriptive regressions flagged, not automatically retuned.'
    (WORK/'results.json').write_text(json.dumps(result,indent=2)+'\n')
    emit({'complete':True,'cases':len(result['cases']),'timed_processes':sum(len(v) for c in result['cases'] for v in c['samples'].values()),'regressions':sum(c['material_regression'] for c in result['cases'])})


if __name__=='__main__':main()
