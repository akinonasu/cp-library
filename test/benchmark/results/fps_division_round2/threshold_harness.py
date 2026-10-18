from pathlib import Path
import os,fcntl,subprocess,time,random,json,statistics,hashlib
W=Path('/tmp/fps-division-round2');F=Path('/tmp/library-checker-problems/polynomial/inv_of_polynomials');values=[8,16,24,32,48,64,96,128,192,256];rng=random.Random(20261022);lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});result={'seed':20261022,'trials':21,'direct_thresholds':values,'baseline_threshold':64,'flags':['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'],'cpu':2,'source_sha256':hashlib.sha256((W/'threshold.cpp').read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256((W/'threshold').read_bytes()).hexdigest(),'workloads':[]}
for case in ['max_random_00','max_random_01','max_random_02']:
 inp=F/'in'/(case+'.in');answer=F/'out'/(case+'.out')
 if not inp.exists():continue
 samples={v:[] for v in values};kernel={v:[] for v in values};orders=[]
 for rep in range(22):
  order=values[:];rng.shuffle(order);orders.append(order)
  for value in order:
   out=W/'threshold.out'
   with inp.open('rb') as fi,out.open('wb') as fo:
    start=time.perf_counter_ns();p=subprocess.run([str(W/'threshold'),str(value)],stdin=fi,stdout=fo,stderr=subprocess.PIPE);elapsed=(time.perf_counter_ns()-start)/1e6
   assert p.returncode==0,(value,p.stderr)
   c=subprocess.run([str(F/'checker'),str(inp),str(out),str(answer)],capture_output=True);assert c.returncode==0,c.stderr
   if rep:samples[value].append(elapsed);kernel[value].append(float(p.stderr))
 item={'case':case,'whole_ms':samples,'kernel_ms':kernel,'orders':orders,'whole_median_ms':{v:statistics.median(s) for v,s in samples.items()},'kernel_median_ms':{v:statistics.median(s) for v,s in kernel.items()}}
 result['workloads'].append(item);(W/'threshold.json').write_text(json.dumps(result,indent=2)+'\n');print(case,item['whole_median_ms'],item['kernel_median_ms'],flush=True)
