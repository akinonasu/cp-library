from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import os,fcntl,subprocess,shutil,time,json,random,statistics,hashlib,platform
R=Path('/workspace/cp-library');W=Path('/tmp/fps-division-round2');B=Path('/tmp/cp-fps-round2-base');C=W/'candidate';O=Path('/tmp/library-checker-problems/polynomial')
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];seed=20261021;trials=21
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_SH)
if C.exists():shutil.rmtree(C)
shutil.copytree(B,C)
p=C/'components/fps_division_blocked.hpp';s=p.read_text();a=s.index('\tstd::size_t i = 0;\n',s.index('inline void append_words'));addition='''\tstd::array<Mint, 64> batch;
\tstd::size_t i = 0;
\tfor (; i + 64 <= n; i += 64) {
\t\tif (factor == 1) std::memcpy(static_cast<void*>(batch.data()), values + i, sizeof(batch));
\t\telse {
#pragma GCC unroll 8
\t\t\tfor (std::size_t j = 0; j < 64; j += 8) {
\t\t\t\tconst Vec word = times(load_any(values + i + j), scale);
\t\t\t\tstd::memcpy(static_cast<void*>(batch.data() + j), &word, sizeof(word));
\t\t\t}
\t\t}
\t\tout.insert(out.end(), batch.begin(), batch.end());
\t}
''';s=s[:a]+s[a:].replace('\tstd::size_t i = 0;\n',addition,1);p.write_text(s)
driver=W/'driver.cpp';shutil.copyfile(R/'test/benchmark/results/fps_division_measurement_driver.cpp',driver)
result={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','baseline_tree':str(B),'candidate_tree':str(C),'flags':flags,'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'cpu':subprocess.check_output(['lscpu'],text=True),'platform':platform.platform(),'seed':seed,'trials':trials,'cpu_affinity':[2],'metric':'whole process wall milliseconds plus computation milliseconds reported separately by driver; computation includes API allocations and output result construction','harness_sha256':sha(Path(__file__)),'sources':{name:{str(p.relative_to(tree)):sha(p) for p in sorted(tree.rglob('*.hpp'))} for name,tree in [('baseline',B),('candidate',C)]},'driver_sha256':sha(driver),'builds':{},'workloads':[]}
jobs=[]
for name,tree in [('baseline',B),('candidate',C)]:
 for mode,label in [(0,'division'),(1,'inverse_mod'),(2,'gcd')]:jobs.append((label+'_'+name,tree,driver,['-DMODE='+str(mode)]))
for label,problem,submission in [('division','division_of_polynomials','409561'),('inverse_mod','inv_of_polynomials','409589')]:jobs.append((label+'_reference',None,Path('/tmp/fps-reference')/problem/(submission+'.cpp'),[]))
def build(job):
 name,tree,source,extra=job;binary=W/name;cmd=['g++',*flags,*extra,*( ['-I',str(tree)] if tree else []),str(source),'-o',str(binary)];t=time.monotonic();r=subprocess.run(cmd,capture_output=True,text=True)
 if r.returncode:raise RuntimeError((name,r.stdout,r.stderr))
 return name,{'command':cmd,'source_sha256':sha(source),'binary_sha256':sha(binary),'compiler_output':r.stdout+r.stderr,'seconds':time.monotonic()-t}
with ThreadPoolExecutor(max_workers=2) as pool:
 for name,info in pool.map(build,jobs):result['builds'][name]=info;print('built',name,round(info['seconds'],2),flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(seed)
cases=[]
for stem in ['v_random_00','v_random_01','v_random_02','v_random_03','n_max_00']:
 cases.append(('division_'+stem,'division','division_of_polynomials',stem))
cases.append(('inverse_max','inverse_mod','inv_of_polynomials','max_random_00'))
cases.append(('gcd_max','gcd',None,'max_random_00'))
def save():(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
for name,label,problem,stem in cases:
 folder=O/(problem or 'inv_of_polynomials');inp=folder/'in'/(stem+'.in');answer=folder/'out'/(stem+'.out');names=['baseline','candidate']+(['reference'] if problem else []);whole={v:[] for v in names};kernel={v:[] for v in names if v!='reference'};expected=None;orders=[]
 for rep in range(trials+1):
  order=names[:];rng.shuffle(order);orders.append(order)
  for variant in order:
   binary=W/(label+'_'+variant);out=W/(name+'_'+variant+'.out')
   with inp.open('rb') as fi,out.open('wb') as fo:
    t=time.perf_counter_ns();p=subprocess.run([str(binary)],stdin=fi,stdout=fo,stderr=subprocess.PIPE,timeout=120);ms=(time.perf_counter_ns()-t)/1e6
   assert p.returncode==0,(name,variant,p.stderr)
   if problem:
    check=subprocess.run([str(folder/'checker'),str(inp),str(out),str(answer)],capture_output=True);assert check.returncode==0,(name,variant,check.stdout,check.stderr)
   else:
    tokens=b' '.join(out.read_bytes().split())
    if expected is None:expected=tokens
    assert tokens==expected
   if rep:
    whole[variant].append(ms)
    if p.stderr:kernel[variant].append(float(p.stderr))
 def summary(samples):
  delta=[b-c for b,c in zip(samples['baseline'],samples['candidate'])]
  return {'samples_ms':samples,'median_ms':{k:statistics.median(v) for k,v in samples.items()},'paired_delta_ms':delta,'paired_median_saving_ms':statistics.median(delta),'candidate_wins':sum(d>0 for d in delta)}
 item={'name':name,'input':str(inp),'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'answer_sha256':sha(answer) if problem else None,'execution_orders_including_warmup':orders,'whole':summary(whole),'kernel':summary(kernel)};result['workloads'].append(item);save();print(name,'whole',item['whole']['median_ms'],'paired',item['whole']['paired_median_saving_ms'],'wins',item['whole']['candidate_wins'],'kernel',item['kernel']['median_ms'],flush=True)
print('complete',flush=True)
