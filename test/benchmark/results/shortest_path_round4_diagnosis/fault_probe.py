import sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
sys.path.insert(0,str(ROOT/'test/benchmark'))
from freeze_sources import freeze, freeze_reference
import fcntl,hashlib,json,os,random,resource,shutil,statistics,subprocess,time
OUT=ROOT/'.verify-helper/cache/shortest-path/round4/reproduce/fault_probe'
OUT.mkdir(parents=True,exist_ok=True)
WORK=Path('/tmp/cp-dijkstra-round4-fault-probe')
WORK.mkdir(exist_ok=True)
os.sched_setaffinity(0,{2})
FLAGS=['-std=gnu++23','-O2','-DNDEBUG','-DATCODER','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=native']
REVISIONS={'f4':'f4aa743','bd':'bd29f70'}
PROBLEM=Path.home()/'.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
LOCK=open('/tmp/cp-wm-opt.lock','w')
TRIALS=41
SEED=2026101905
bins={}
env={'flags':FLAGS,'revisions':REVISIONS,'affinity':[2],'trials':TRIALS,'order_seed':SEED,'sources':{},'thp':Path('/sys/kernel/mm/transparent_hugepage/enabled').read_text(),'aslr':Path('/proc/sys/kernel/randomize_va_space').read_text()}
for name,revision in REVISIONS.items():
    snapshot=OUT/'sources'/('own_'+name)
    translation,copied,manifest=freeze_reference(snapshot,ROOT/'test/dijkstra_path.test.cpp',revision)
    env['sources']['own_'+name]={'hashes':copied,'manifest':manifest}
    bins['own_'+name]=WORK/('own_'+name)
    fcntl.flock(LOCK,fcntl.LOCK_EX)
    try: subprocess.run(['g++',*FLAGS,str(translation),'-o',str(bins['own_'+name])],check=True)
    finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
    driver=subprocess.check_output(['git','show','bd29f70:test/benchmark/shortest_path_round3.cpp'],cwd=ROOT,text=True)
    driver=driver.replace('#include <array>','#include <array>\n#include <sys/resource.h>')
    driver=driver.replace('auto start = Clock::now();','rusage before{}, parsed_usage{}, built_usage{};\n\tgetrusage(RUSAGE_SELF, &before);\n\tauto start = Clock::now();')
    driver=driver.replace('parsed = Clock::now();','getrusage(RUSAGE_SELF, &parsed_usage);\n\t\tparsed = Clock::now();')
    driver=driver.replace('auto built = Clock::now();','auto built = Clock::now();\n\tgetrusage(RUSAGE_SELF, &built_usage);')
    driver=driver.rstrip()[:-1]+'''\n\tstd::fprintf(stderr, "{\\"offset_addr\\":%llu,\\"arcs_addr\\":%llu,\\"edges_addr\\":%llu,\\"parse_minor\\":%ld,\\"build_minor\\":%ld,\\"build_major\\":%ld}\\n", static_cast<unsigned long long>(reproduce::offset), static_cast<unsigned long long>(reproduce::arcs), static_cast<unsigned long long>(reproduce::edges), parsed_usage.ru_minflt-before.ru_minflt, built_usage.ru_minflt-parsed_usage.ru_minflt, built_usage.ru_majflt-parsed_usage.ru_majflt);\n}\n'''
    snapshot=OUT/'sources'/('phase_'+name)
    env['sources']['phase_'+name]=freeze(snapshot,driver,ROOT,revision)
    csr=snapshot/'components/directed_csr.hpp'
    text=csr.read_text().replace('template<class T>\nclass DirectedCSR', 'namespace reproduce { inline std::uintptr_t offset, arcs, edges; }\n\ntemplate<class T>\nclass DirectedCSR')
    text=text.replace('arcs_.resize(edges.size());','arcs_.resize(edges.size());\n\t\treproduce::offset = reinterpret_cast<std::uintptr_t>(offset_.data()); reproduce::arcs = reinterpret_cast<std::uintptr_t>(arcs_.data()); reproduce::edges = reinterpret_cast<std::uintptr_t>(edges.data());')
    csr.write_text(text)
    (snapshot/'driver.cpp').write_text(driver)
    bins['phase_'+name]=WORK/('phase_'+name)
    fcntl.flock(LOCK,fcntl.LOCK_EX)
    try: subprocess.run(['g++',*FLAGS,*(['-DSP_BASELINE=1'] if name=='f4' else []),'-I',str(snapshot),str(snapshot/'driver.cpp'),'-o',str(bins['phase_'+name])],check=True)
    finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
(OUT/'environment.json').write_text(json.dumps(env,indent=2)+'\n')
rng=random.Random(SEED)
case='max_sparse_random_01'
inp=WORK/'input.in'
shutil.copyfile(PROBLEM/'in'/(case+'.in'),inp)
record={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'samples':{},'trace':[]}
expected=None
fcntl.flock(LOCK,fcntl.LOCK_EX)
try:
    for trial in range(-1,TRIALS):
        order=list(bins)
        rng.shuffle(order)
        for position,name in enumerate(order):
            out=WORK/'answer.out'
            before=resource.getrusage(resource.RUSAGE_CHILDREN)
            with inp.open('rb') as fin,out.open('wb') as fout:
                started=time.perf_counter()
                completed=subprocess.run([bins[name]],stdin=fin,stdout=fout,stderr=subprocess.PIPE,check=True)
                elapsed=(time.perf_counter()-started)*1000
            after=resource.getrusage(resource.RUSAGE_CHILDREN)
            value=json.loads(completed.stderr.splitlines()[0]) if name.startswith('phase') else {'distance':int(out.read_bytes().split(maxsplit=1)[0])}
            if name.startswith('phase'): value.update(json.loads(completed.stderr.splitlines()[1]))
            if expected is None: expected=value['distance']
            assert expected==value['distance']
            if trial==-1:
                subprocess.run([PROBLEM/'checker',inp,out,PROBLEM/'out'/(case+'.out')],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
            else:
                value.update(whole_ms=elapsed,user_ms=(after.ru_utime-before.ru_utime)*1000,system_ms=(after.ru_stime-before.ru_stime)*1000,minor_fault=after.ru_minflt-before.ru_minflt,major_fault=after.ru_majflt-before.ru_majflt,voluntary_switch=after.ru_nvcsw-before.ru_nvcsw,involuntary_switch=after.ru_nivcsw-before.ru_nivcsw,loadavg=os.getloadavg())
                record['samples'].setdefault(name,[]).append(value)
                record['trace'].append({'trial':trial,'position':position,'name':name,**value})
finally: fcntl.flock(LOCK,fcntl.LOCK_UN)
record['summary']={}
for name,samples in record['samples'].items():
    record['summary'][name]={}
    for key in ['whole_ms','parse_ms','build_ms','solve_ms','output_ms','minor_fault','build_minor','user_ms','system_ms','involuntary_switch']:
        if key in samples[0]:
            values=sorted(row[key] for row in samples)
            record['summary'][name][key]={'min':min(values),'median':statistics.median(values),'p25':values[TRIALS//4],'p75':values[3*TRIALS//4],'max':max(values)}
(OUT/'results.json').write_text(json.dumps(record,indent=2)+'\n')
print({name:round(data['whole_ms']['median'],3) for name,data in record['summary'].items()},flush=True)
for name,samples in record['samples'].items():
    if name.startswith('phase'):
        print(name,[(round(row['build_ms'],3),row['build_minor'],hex(row['arcs_addr']%2097152),row['minor_fault']) for row in samples],flush=True)
