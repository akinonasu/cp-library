import concurrent.futures,datetime,fcntl,hashlib,json,pathlib,random,resource,statistics,subprocess,time
work=pathlib.Path(__file__).resolve().parent;base=work.parent/'final-benchmark';repo=pathlib.Path('/workspace/cp-library')
sha=lambda p:hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()
flags=['-std=gnu++23','-O2','-march=native','-ftrivial-auto-var-init=zero','-UNDEBUG','-fno-lto','-Wall','-Wextra','-Werror']
old=json.loads((base/'results.json').read_text())
shapes=['uniform_same','random30','duplicates16','short','no_op_random'];cases=[json.loads((work.parent/'cases-large'/f'{s}.json').read_text()) for s in shapes]
result={'utc':str(datetime.datetime.now(datetime.timezone.utc)),'flags':flags,'cpu':2,'repeats':3,'warmups':1,'seed':20261020,'sources':{},'binaries':{},'commands':[],'validation':[],'samples':[],'warmup_samples':[],'summary':{}}
for p in ['components/dynamic_quad_vector.hpp','components/dynamic_quad_wavelet.hpp','components/dynamic_bit_vector.hpp','data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp']:result['sources'][p]=sha(work/p)
result['driver_sha256']=sha(base/'sources/abc343f.cpp')
for tag in ['baseline_stream','final_stream','baseline_api','final_api']:
 b=old['metadata']['binaries'][tag];assert sha(b['path'])==b['sha256'];result['binaries'][tag]={'path':b['path'],'sha256':b['sha256'],'family':b['family']}
def compile_one(family):
 tag='candidate_'+family;target=work/tag;cmd=['g++',*flags,*(['-DDWM_INSTRUMENT=1'] if family=='api' else []),'-I'+str(work),'-I'+str(repo),str(base/'sources/abc343f.cpp'),'-o',str(target)]
 with open('/tmp/cp-fps-tuning.lock','w') as lock:
  fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run(cmd,check=True)
 print('compiled',tag,flush=True)
 return tag,{'path':str(target),'sha256':sha(target),'family':family},cmd
with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
 for tag,data,command in pool.map(compile_one,['stream','api']):result['binaries'][tag]=data;result['commands'].append(command)
for c in cases:
 assert sha(c['input']['path'])==c['input']['sha256'];assert sha(c['expected']['path'])==c['expected']['sha256']
def cpu():
 r=resource.getrusage(resource.RUSAGE_CHILDREN);return r.ru_utime+r.ru_stime
def run(case,tag,trial):
 b=result['binaries'][tag];output=work/'screen-output.txt';cmd=['taskset','-c','2',b['path']]
 with open(case['input']['path'],'rb') as f,output.open('wb') as out:
  w,c=time.perf_counter(),cpu();r=subprocess.run(cmd,stdin=f,stdout=out,stderr=subprocess.PIPE,check=True,timeout=120);w,c=time.perf_counter()-w,cpu()-c
 digest=sha(output);assert digest==case['expected']['sha256'],(case['name'],tag)
 return {'case':case['name'],'variant':tag,'trial':trial,'wall_seconds':w,'cpu_seconds':c,'answer_sha256':digest,'api':json.loads(r.stderr) if b['family']=='api' else None}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 small=[json.loads(p.read_text()) for p in sorted((work.parent/'cases-small').glob('small*.json'))]
 for case in small:
  for tag in ['candidate_stream','candidate_api']:result['validation'].append(run(case,tag,-2))
 print('small preflight passed',flush=True)
# Caller starts measurement only after independent correctness workers have exited.
assert (work/'validation/manifest.json').exists(),'Five generic unit validation must finish before timing'
for gate_attempt in range(60):
 if (work/'uniform_check.passed').exists():break
 time.sleep(1)
assert (work/'uniform_check.passed').exists(),'Focused uniform validation must finish before timing'
rng=random.Random(20261020)
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('acquired EX',flush=True)
 rng.shuffle(cases)
 for case in cases:
  tags=list(result['binaries']);rng.shuffle(tags)
  for tag in tags:result['warmup_samples'].append(run(case,tag,-1))
  for trial in range(3):
   tags=list(result['binaries']);rng.shuffle(tags)
   for tag in tags:result['samples'].append(run(case,tag,trial))
  print('timed',case['name'],flush=True)
  (work/'screen.json').write_text(json.dumps(result,indent=2)+'\n')
for case in cases:
 table={}
 for tag in result['binaries']:
  rows=[r for r in result['samples'] if r['case']==case['name'] and r['variant']==tag]
  metrics={key:statistics.median([r[key] for r in rows]) for key in ['wall_seconds','cpu_seconds']}
  if rows[0]['api']:
   for key in ['build_cpu','build_wall','operation_cpu','operation_wall']:metrics[key]=statistics.median([r['api'][key] for r in rows])
  table[tag]=metrics
 result['summary'][case['name']]=table
for p,h in result['sources'].items():assert sha(work/p)==h
for tag,b in result['binaries'].items():assert sha(b['path'])==b['sha256']
(work/'screen.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result['summary'],indent=2),flush=True)
