import sys,subprocess
from pathlib import Path
ROOT=Path(subprocess.check_output(['git','rev-parse','--show-toplevel'],text=True).strip())
sys.path.insert(0,str(ROOT/'test/benchmark'))
from shortest_path_round3 import freeze, freeze_reference
import fcntl,hashlib,json,os,random,resource,shutil,statistics,subprocess,tempfile,time
OUT=Path(__file__).resolve().parent/('generic' if '--generic' in sys.argv else 'native')
OUT.mkdir(parents=True,exist_ok=True)
os.sched_setaffinity(0,{2})
FLAGS=['-std=gnu++23','-O2','-DNDEBUG','-DATCODER','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero']+([] if '--generic' in sys.argv else ['-march=native'])
REVISIONS={'bd':'bd29f70','c075':'c075fb7'}
CASES=['max_dense_random_01','max_dense_random_00','max_dense_zero_00','max_dense_long_00','max_sparse_random_01','line_00','max_star_00']
PROBLEM=Path.home()/'.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
LOCK=open('/tmp/cp-wm-opt.lock','w')
TRIALS=21 if '--generic' in sys.argv else 31
SEED=2026101905
with tempfile.TemporaryDirectory(prefix='cp-dijkstra-reproduce-') as temp:
    work=Path(temp)
    bins={}
    env={'flags':FLAGS,'revisions':REVISIONS,'affinity':[2],'trials':TRIALS,'order_seed':SEED,'compiler':subprocess.check_output(['g++','--version'],text=True),'sources':{}}
    for name,revision in REVISIONS.items():
        snapshot=OUT/'sources'/('own_'+name)
        translation,copied,manifest=freeze_reference(snapshot,ROOT/'test/dijkstra_path.test.cpp',revision)
        env['sources']['own_'+name]={'hashes':copied,'manifest':manifest}
        bins['own_'+name]=work/('own_'+name)
        fcntl.flock(LOCK,fcntl.LOCK_EX)
        try: subprocess.run(['g++',*FLAGS,str(translation),'-o',str(bins['own_'+name])],check=True)
        finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
        driver=subprocess.check_output(['git','show','c075fb7:test/benchmark/shortest_path_round4.cpp'],cwd=ROOT,text=True)
        snapshot=OUT/'sources'/('phase_'+name)
        env['sources']['phase_'+name]=freeze(snapshot,driver,ROOT,revision)
        (snapshot/'driver.cpp').write_text(driver)
        bins['phase_'+name]=work/('phase_'+name)
        fcntl.flock(LOCK,fcntl.LOCK_EX)
        try: subprocess.run(['g++',*FLAGS,'-I',str(snapshot),str(snapshot/'driver.cpp'),'-o',str(bins['phase_'+name])],check=True)
        finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
    (OUT/'environment.json').write_text(json.dumps(env,indent=2)+'\n')
    rng=random.Random(SEED)
    results={}
    for case in CASES:
        inp=work/'input.in'
        shutil.copyfile(PROBLEM/'in'/(case+'.in'),inp)
        record={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'samples':{},'trace':[]}
        expected=None
        fcntl.flock(LOCK,fcntl.LOCK_EX)
        try:
            for trial in range(-1,TRIALS):
                order=list(bins)
                rng.shuffle(order)
                for position,name in enumerate(order):
                    out=work/'answer.out'
                    before=resource.getrusage(resource.RUSAGE_CHILDREN)
                    with inp.open('rb') as fin,out.open('wb') as fout:
                        started=time.perf_counter()
                        completed=subprocess.run([bins[name]],stdin=fin,stdout=fout,stderr=subprocess.PIPE,check=True)
                        elapsed=(time.perf_counter()-started)*1000
                    after=resource.getrusage(resource.RUSAGE_CHILDREN)
                    value=json.loads(completed.stderr) if name.startswith('phase') else {'distance':int(out.read_bytes().split(maxsplit=1)[0])}
                    if expected is None: expected=value['distance']
                    assert expected==value['distance']
                    if trial==-1:
                        subprocess.run([PROBLEM/'checker',inp,out,PROBLEM/'out'/(case+'.out')],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
                    else:
                        value.update(whole_ms=elapsed,user_ms=(after.ru_utime-before.ru_utime)*1000,system_ms=(after.ru_stime-before.ru_stime)*1000,voluntary_switch=after.ru_nvcsw-before.ru_nvcsw,involuntary_switch=after.ru_nivcsw-before.ru_nivcsw,loadavg=os.getloadavg())
                        record['samples'].setdefault(name,[]).append(value)
                        record['trace'].append({'trial':trial,'position':position,'name':name,**value})
        finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
        record['summary']={}
        for name,samples in record['samples'].items():
            record['summary'][name]={}
            for key in ['whole_ms','parse_ms','build_ms','solve_ms','output_ms','user_ms','system_ms','involuntary_switch']:
                if key in samples[0]:
                    values=sorted(row[key] for row in samples)
                    quartiles=statistics.quantiles(values,n=4,method='inclusive')
                    record['summary'][name][key]={'min':min(values),'median':statistics.median(values),'p25':quartiles[0],'p75':quartiles[2],'max':max(values)}
        results[case]=record
        (OUT/'results.json').write_text(json.dumps(results,indent=2)+'\n')
        print(case,{name:round(data['whole_ms']['median'],3) for name,data in record['summary'].items()},flush=True)
    print('All paths checked',flush=True)
