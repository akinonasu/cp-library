from pathlib import Path
import subprocess,fcntl,json,hashlib,random,statistics,time,os
W=Path('/tmp/fps-evaluation-round3');os.sched_setaffinity(0,{2});sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
bins={x:W/(x+'.out')for x in ['base','split','split_noodd','reference']};r=dict(baseline='bd951dc',trials=21,cpu=2,seed=20261024,binary_sha256={x:sha(b)for x,b in bins.items()},harness_sha256=sha(Path(__file__)),results=[])
r['flags']=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];r['headers']={x:sha(W/x/'components/fps_taylor_blocked.hpp')for x in ['base','split','split_noodd']}
J=Path('/tmp/library-checker-problems/polynomial/polynomial_taylor_shift')
with open('/tmp/cp-fps-tuning.lock','w')as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 for variant in ['split','split_noodd']:
  for inp in sorted((J/'in').glob('*.in')):
   out=W/'split.out.actual'
   with inp.open('rb')as src,out.open('wb')as dst:subprocess.run([str(bins[variant])],stdin=src,stdout=dst,check=True)
   subprocess.run([str(J/'checker'),str(inp),str(out),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 fcntl.flock(lock,fcntl.LOCK_UN);print('checked both38',flush=True)
 fcntl.flock(lock,fcntl.LOCK_EX);rng=random.Random(20261024)
 for fn in ['max_random_00.in','max_random_01.in']:
  inp=J/'in'/fn;samples={x:[]for x in bins};orders=[]
  for tr in range(22):
   order=list(bins);rng.shuffle(order);orders.append(order)
   for name in order:
    out=W/'split.out.actual'
    with inp.open('rb')as src,out.open('wb')as dst:
     t=time.perf_counter_ns();subprocess.run([str(bins[name])],stdin=src,stdout=dst,check=True);elapsed=(time.perf_counter_ns()-t)/1e6
    if tr:samples[name].append(elapsed)
    subprocess.run([str(J/'checker'),str(inp),str(out),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  row=dict(input=fn,input_sha256=sha(inp),orders=orders,samples_ms=samples,median_ms={x:statistics.median(a)for x,a in samples.items()},paired_saving_ms={x:statistics.median([a-b for a,b in zip(samples['base'],samples[x])])for x in ['split','split_noodd']},wins={x:sum(a>b for a,b in zip(samples['base'],samples[x]))for x in ['split','split_noodd']})
  r['results'].append(row);print(fn,row['median_ms'],row['paired_saving_ms'],row['wins'],flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
(W/'split_run.json').write_text(json.dumps(r,indent=2)+'\n')
