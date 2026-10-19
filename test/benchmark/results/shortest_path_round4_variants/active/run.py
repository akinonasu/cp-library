import sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[5]
sys.path.insert(0,str(ROOT/'test/benchmark'))
from freeze_sources import freeze,freeze_reference
from shortest_path import check_synthetic
import fcntl,hashlib,json,os,random,re,resource,shutil,statistics,subprocess,tempfile,time
OUT=ROOT/'.verify-helper/cache/shortest-path/round4/active'
OUT.mkdir(parents=True,exist_ok=True)
os.sched_setaffinity(0,{2})
FLAGS=['-std=gnu++23','-O2','-DNDEBUG','-DATCODER','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=native']
NAMES=['bd','current','active','deferred','original_active']
CASES=['max_star_00','max_star_01','max_sparse_random_01','line_00','grid_swirl_00','max_dense_random_00','synthetic_line']
PROBLEM=Path.home()/'.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
LOCK=open('/tmp/cp-wm-opt.lock','w')
TRIALS=9
SEED=2026101910
def change(text,name):
    patch=(Path(__file__).parent/'patches'/(name+'.patch')).read_bytes()
    original=subprocess.check_output(['git','show','bd29f70:components/directed_csr.hpp'],cwd=ROOT)
    if not patch.strip(): return original.decode()
    with tempfile.TemporaryDirectory(prefix='cp-dijkstra-active-rebuild-') as temp:
        work=Path(temp)
        (work/'components').mkdir()
        target=work/'components/directed_csr.hpp'
        target.write_bytes(original)
        subprocess.run(['patch','-p1','--quiet'],input=patch,cwd=work,check=True)
        return target.read_text()
env={'flags':FLAGS,'affinity':[2],'trials':TRIALS,'seed':SEED,'variants':NAMES,'sources':{}}
with tempfile.TemporaryDirectory(prefix='cp-dijkstra-active-') as temp:
    work=Path(temp)
    bins={}
    for name in NAMES:
        revision='bd29f70'
        snapshot=OUT/'sources'/('own_'+name)
        translation,copied,manifest=freeze_reference(snapshot,ROOT/'test/dijkstra_path.test.cpp',revision)
        csr=snapshot/manifest['components/directed_csr.hpp']
        csr.write_text(change(csr.read_text(),name))
        env['sources']['own_'+name]={'revision':revision,'origin_sha256':copied,'manifest':manifest}
        bins['own_'+name]=work/('own_'+name)
        fcntl.flock(LOCK,fcntl.LOCK_EX)
        try: subprocess.run(['g++',*FLAGS,str(translation),'-o',str(bins['own_'+name])],check=True)
        finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
        driver=(Path(__file__).parent/'driver.cpp').read_text()
        snapshot=OUT/'sources'/('phase_'+name)
        env['sources']['phase_'+name]=freeze(snapshot,driver,ROOT,revision)
        csr=snapshot/'components/directed_csr.hpp'
        csr.write_text(change(csr.read_text(),name))
        (snapshot/'driver.cpp').write_text(driver)
        bins['phase_'+name]=work/('phase_'+name)
        fcntl.flock(LOCK,fcntl.LOCK_EX)
        try: subprocess.run(['g++',*FLAGS,'-I',str(snapshot),str(snapshot/'driver.cpp'),'-o',str(bins['phase_'+name])],check=True)
        finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
    env['frozen_sha256']={str(file.relative_to(OUT/'sources')):hashlib.sha256(file.read_bytes()).hexdigest() for file in (OUT/'sources').rglob('*') if file.is_file()}
    (OUT/'environment.json').write_text(json.dumps(env,indent=2)+'\n')
    rng=random.Random(SEED)
    results={}
    for case in CASES:
        inp=work/'input.in'
        source=(ROOT/'.verify-helper/cache/shortest-path/round3/inputs'/(case+'.in')) if case.startswith('synthetic') else PROBLEM/'in'/(case+'.in')
        shutil.copyfile(source,inp)
        record={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'samples':{},'trace':[]}
        expected=None
        synthetic=None
        fcntl.flock(LOCK,fcntl.LOCK_EX)
        try:
            for trial in range(-1,TRIALS):
                order=list(bins)
                rng.shuffle(order)
                for position,name in enumerate(order):
                    out=work/'answer.out'
                    with inp.open('rb') as fin,out.open('wb') as fout:
                        started=time.perf_counter()
                        completed=subprocess.run([bins[name]],stdin=fin,stdout=fout,stderr=subprocess.PIPE,check=True)
                        elapsed=(time.perf_counter()-started)*1000
                    value=json.loads(completed.stderr) if name.startswith('phase') else {'distance':int(out.read_bytes().split(maxsplit=1)[0])}
                    if expected is None: expected=value['distance']
                    assert expected==value['distance']
                    if trial==-1:
                        if case.startswith('synthetic'): synthetic=check_synthetic(inp,out,expected,synthetic)
                        else: subprocess.run([PROBLEM/'checker',inp,out,PROBLEM/'out'/(case+'.out')],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
                    else:
                        value['whole_ms']=elapsed
                        record['samples'].setdefault(name,[]).append(value)
                        record['trace'].append({'trial':trial,'position':position,'name':name,**value})
        finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
        record['median_ms']={name:{key:statistics.median(row[key] for row in samples) for key in samples[0] if key.endswith('_ms')} for name,samples in record['samples'].items()}
        results[case]=record
        (OUT/'results.json').write_text(json.dumps(results,indent=2)+'\n')
        print(case,{name:round(data['whole_ms'],3) for name,data in record['median_ms'].items()},flush=True)
    print('All paths checked',flush=True)
