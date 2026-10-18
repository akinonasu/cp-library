from pathlib import Path
import os,fcntl,subprocess,time,random,json,statistics,hashlib
W=Path('/tmp/fps-division-round2');F=Path('/tmp/library-checker-problems/polynomial/inv_of_polynomials');rng=random.Random(20261024);lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});result={'seed':20261024,'trials':21,'cpu_affinity':[2],'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'build':json.loads((W/'fused_build.json').read_text()),'workloads':[]}
variants={'baseline':'inverse_mod_baseline','candidate':'inverse_mod_fused','reference':'inverse_mod_reference'}
for case in ['max_random_00','max_random_01','max_random_02','abnormal_random_00','abnormal_random_01']:
 inp=F/'in'/(case+'.in');answer=F/'out'/(case+'.out');samples={v:[] for v in variants};kernel={v:[] for v in variants if v!='reference'};orders=[]
 for rep in range(22):
  order=list(samples);rng.shuffle(order);orders.append(order)
  for variant in order:
   out=W/'fused.out'
   with inp.open('rb') as fi,out.open('wb') as fo:
    start=time.perf_counter_ns();p=subprocess.run([str(W/variants[variant])],stdin=fi,stdout=fo,stderr=subprocess.PIPE);elapsed=(time.perf_counter_ns()-start)/1e6
   assert p.returncode==0,p.stderr
   c=subprocess.run([str(F/'checker'),str(inp),str(out),str(answer)],capture_output=True);assert c.returncode==0,(variant,c.stderr)
   if rep:
    samples[variant].append(elapsed)
    if p.stderr:kernel[variant].append(float(p.stderr))
 delta=[b-c for b,c in zip(samples['baseline'],samples['candidate'])]
 item={'case':case,'input_header':inp.open().readline().strip(),'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'answer_sha256':hashlib.sha256(answer.read_bytes()).hexdigest(),'whole_ms':samples,'kernel_ms':kernel,'orders':orders,'whole_median_ms':{v:statistics.median(s) for v,s in samples.items()},'kernel_median_ms':{v:statistics.median(s) for v,s in kernel.items()},'paired_saving_ms':delta,'paired_median_saving_ms':statistics.median(delta),'candidate_wins':sum(d>0 for d in delta)}
 result['workloads'].append(item);(W/'fused_results.json').write_text(json.dumps(result,indent=2)+'\n');print(case,item['whole_median_ms'],item['kernel_median_ms'],'paired',item['paired_median_saving_ms'],'wins',item['candidate_wins'],flush=True)
