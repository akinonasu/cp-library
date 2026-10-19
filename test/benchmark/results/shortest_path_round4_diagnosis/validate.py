from pathlib import Path
import ast,fcntl,hashlib,json,re,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[4]
BASE=ROOT/'test/benchmark/results/shortest_path_round4_diagnosis'
SRC=ROOT/'.verify-helper/cache/shortest-path/round4/reproduce'
def sha(data): return hashlib.sha256(data.replace(b'\r\n',b'\n')).hexdigest()
manifest=json.loads((BASE/'source_manifest.json').read_text())
check={'python_syntax':[],'trial_counts':{},'original_hashes':0,'frozen_hashes':0,'patches':0,'drivers':0}
with open('/tmp/cp-wm-opt.lock','w') as lock:
    fcntl.flock(lock,fcntl.LOCK_EX)
    for script in BASE.glob('*.py'):
        ast.parse(script.read_text())
        check['python_syntax'].append(script.name)
    for mode,record in manifest.items():
        folder=SRC if mode=='exact' else SRC/(mode+'_probe')
        for relative,digest in record['files'].items():
            assert sha((folder/'sources'/relative).read_bytes())==digest
            check['frozen_hashes']+=1
        env=json.loads((BASE/'runs'/mode/'environment.json').read_text())
        for variant,source in env['sources'].items():
            revision='f4aa743' if variant.endswith('f4') else 'bd29f70'
            hashes=source.get('hashes',source)
            for relative,digest in hashes.items():
                data=subprocess.check_output(['git','show',revision+':'+relative],cwd=ROOT)
                assert sha(data)==digest,(mode,variant,relative)
                check['original_hashes']+=1
        results=json.loads((BASE/'runs'/mode/'results.json').read_text())
        cases=results.values() if mode=='exact' else [results]
        count=15 if mode=='exact' else 41
        for case in cases:
            assert all(len(rows)==count for rows in case['samples'].values())
        check['trial_counts'][mode]=count
        for variant,detail in record['diagnostic_patches'].items():
            with tempfile.TemporaryDirectory(prefix='cp-dijkstra-diag-patch-') as temp:
                work=Path(temp)
                (work/'components').mkdir()
                original=subprocess.check_output(['git','show',detail['base']+':components/directed_csr.hpp'],cwd=ROOT)
                (work/'components/directed_csr.hpp').write_bytes(original.replace(b'\r\n',b'\n'))
                patch=(BASE/detail['patch']).read_bytes()
                assert sha(patch)==detail['patch_sha256']
                subprocess.run(['patch','-p1','--quiet'],input=patch,cwd=work,check=True)
                key=variant+('/components/directed_csr.hpp' if variant.startswith('phase') else '/6.hpp')
                assert sha((work/'components/directed_csr.hpp').read_bytes())==record['files'][key]
                check['patches']+=1
            if 'driver' in detail:
                data=(BASE/detail['driver']).read_bytes()
                assert sha(data)==detail['driver_sha256']==record['files'][variant+'/driver.cpp']
                assert not any(value.startswith((b'/', b'C:')) for value in re.findall(rb'#include\s+"([^"]+)"',data))
                check['drivers']+=1
    fcntl.flock(lock,fcntl.LOCK_UN)
check['status']='passed'
(BASE/'validation.json').write_text(json.dumps(check,indent=2)+'\n')
print(check)
