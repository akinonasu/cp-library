from pathlib import Path
import os,fcntl,subprocess,time,random,json,statistics,hashlib
W=Path('/tmp/fps-division-round2');F=Path('/tmp/library-checker-problems/polynomial');rng=random.Random(20261023);lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});result={'seed':20261023,'trials':21,'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'first_results_sha256':hashlib.sha256((W/'results.json').read_bytes()).hexdigest(),'workloads':[]}
for label,problem,case in [('division','division_of_polynomials','v_random_01'),('division','division_of_polynomials','v_random_02'),('division','division_of_polynomials','v_random_03'),('inverse_mod','inv_of_polynomials','max_random_00'),('gcd','inv_of_polynomials','max_random_00')]:
 folder=F/problem;inp=folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out');samples={v:[] for v in ['baseline','candidate']};kernel={v:[] for v in samples};orders=[]
 for rep in range(22):
  order=list(samples);rng.shuffle(order);orders.append(order)
  for variant in order:
   out=W/'repeat.out'
   with inp.open('rb') as fi,out.open('wb') as fo:
    start=time.perf_counter_ns();p=subprocess.run([str(W/(label+'_'+variant))],stdin=fi,stdout=fo,stderr=subprocess.PIPE);elapsed=(time.perf_counter_ns()-start)/1e6
   assert p.returncode==0,p.stderr
   if label!='gcd':
    c=subprocess.run([str(folder/'checker'),str(inp),str(out),str(answer)],capture_output=True);assert c.returncode==0,c.stderr
   else:assert out.read_bytes().split()==[b'1',b'1']
   if rep:samples[variant].append(elapsed);kernel[variant].append(float(p.stderr))
 delta=[b-c for b,c in zip(samples['baseline'],samples['candidate'])]
 item={'label':label,'case':case,'whole_ms':samples,'kernel_ms':kernel,'orders':orders,'whole_median_ms':{v:statistics.median(s) for v,s in samples.items()},'kernel_median_ms':{v:statistics.median(s) for v,s in kernel.items()},'paired_saving_ms':delta,'paired_median_saving_ms':statistics.median(delta),'candidate_wins':sum(d>0 for d in delta)}
 result['workloads'].append(item);(W/'repeat.json').write_text(json.dumps(result,indent=2)+'\n');print(label,case,item['whole_median_ms'],item['kernel_median_ms'],'paired',item['paired_median_saving_ms'],'wins',item['candidate_wins'],flush=True)
