from pathlib import Path
from datetime import datetime,timezone
import os,fcntl,subprocess,time,random,json,statistics,hashlib,platform
W=Path('/tmp/fps-division-round2');B=Path('/tmp/cp-fps-round2-base');C=W/'dividend_scoped_candidate';O=Path('/tmp/library-checker-problems/polynomial');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();seed=20261028;rng=random.Random(seed)
check=json.loads((W/'final_check.json').read_text());assert check.get('headers_unchanged') and all(v['returncode']==0 for v in check['runs'].values())
j={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','started_at':datetime.now(timezone.utc).isoformat(),'seed':seed,'trials':21,'cpu_affinity':[2],'platform':platform.platform(),'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'hardware':subprocess.check_output(['lscpu'],text=True),'metric':'whole-process wall milliseconds including process creation, input/output, allocations, computation, and exit; computation milliseconds from identical API driver excludes I/O and includes allocations/result construction','harness_sha256':sha(Path(__file__)),'driver_sha256':sha(W/'driver.cpp'),'baseline_builds':json.loads((W/'results.json').read_text())['builds'],'candidate_builds':check['builds'],'series_baseline_build':json.loads((W/'series_build.json').read_text())['series_round2_baseline'],'headers_before':{variant:{str(p.relative_to(tree)):sha(p) for p in sorted(tree.rglob('*.hpp'))} for variant,tree in [('baseline',B),('candidate',C)]},'workloads':[]}
lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2})
cases=[('division','division_of_polynomials',s) for s in ['v_random_00','v_random_01','v_random_02','v_random_03','n_max_00']]+[('inverse_mod','inv_of_polynomials',s) for s in ['max_random_00','max_random_01','max_random_02']]+[('gcd',None,'max_random_00'),('series',None,'max')]
for label,problem,case in cases:
 folder=O/(problem or 'inv_of_polynomials');inp=Path('/tmp/fps-division/series.in') if label=='series' else folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out') if problem else None
 variants={'baseline':label+'_baseline','candidate':label+'_final'}
 if label=='series':variants['baseline']='series_round2_baseline'
 if problem:variants['reference']=label+'_reference'
 whole={v:[] for v in variants};kernel={v:[] for v in variants if v!='reference'};orders=[];expected=None;outputs={}
 for rep in range(22):
  order=list(variants);rng.shuffle(order);orders.append(order)
  for variant in order:
   out=W/(label+'_final_actual.out')
   with inp.open('rb') as fi,out.open('wb') as fo:
    start=time.perf_counter_ns();p=subprocess.run([str(W/variants[variant])],stdin=fi,stdout=fo,stderr=subprocess.PIPE,timeout=120);elapsed=(time.perf_counter_ns()-start)/1e6
   assert p.returncode==0,(label,case,variant,p.stderr)
   if problem:
    cmd=[str(folder/'checker'),str(inp),str(out),str(answer)];v=subprocess.run(cmd,capture_output=True);assert v.returncode==0,(cmd,v.stderr)
   else:
    tokens=b' '.join(out.read_bytes().split());digest=hashlib.sha256(tokens).hexdigest()
    if expected is None:expected=digest
    assert digest==expected,(label,variant);outputs[variant]=digest
   if rep:
    whole[variant].append(elapsed)
    if p.stderr:kernel[variant].append(float(p.stderr))
 def summarize(samples):
  delta=[b-c for b,c in zip(samples['baseline'],samples['candidate'])]
  return {'samples_ms':samples,'median_ms':{v:statistics.median(s) for v,s in samples.items()},'paired_saving_ms':delta,'paired_median_saving_ms':statistics.median(delta),'candidate_wins':sum(d>0 for d in delta)}
 item={'label':label,'case':case,'input':str(inp),'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'answer_sha256':sha(answer) if answer else None,'checker_sha256':sha(folder/'checker') if problem else None,'orders_including_warmup':orders,'whole':summarize(whole),'kernel':summarize(kernel),'output_tokens_sha256':outputs};j['workloads'].append(item);(W/'final_results.json').write_text(json.dumps(j,indent=2)+'\n');print(label,case,'whole',item['whole']['median_ms'],'kernel',item['kernel']['median_ms'],'paired',item['whole']['paired_median_saving_ms'],'wins',item['whole']['candidate_wins'],flush=True)
j['headers_after']={variant:{str(p.relative_to(tree)):sha(p) for p in sorted(tree.rglob('*.hpp'))} for variant,tree in [('baseline',B),('candidate',C)]};j['headers_unchanged']=j['headers_before']==j['headers_after'];assert j['headers_unchanged'];j['finished_at']=datetime.now(timezone.utc).isoformat();(W/'final_results.json').write_text(json.dumps(j,indent=2)+'\n');print('final measurement complete',flush=True)
