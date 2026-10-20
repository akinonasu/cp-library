from pathlib import Path
import subprocess,fcntl,json,random,statistics,time,os,re,hashlib
W=Path('/tmp/fps-evaluation-round3');os.sched_setaffinity(0,{2});J=Path('/tmp/library-checker-problems/polynomial/polynomial_taylor_shift');r=dict(results=[],trials=11)
with open('/tmp/cp-fps-tuning.lock','w')as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);rng=random.Random(19)
 for fn in ['max_random_00.in','max_random_01.in']:
  inp=J/'in'/fn;samples={x:[]for x in ['base','reference']}
  for tr in range(12):
   order=list(samples);rng.shuffle(order)
   for name in order:
    out=W/'profile.out'
    with inp.open('rb')as src,out.open('wb')as dst:
     begin=time.perf_counter_ns();p=subprocess.run([str(W/('profile_'+name+'.out'))],stdin=src,stdout=dst,stderr=subprocess.PIPE,check=True,text=True);elapsed=time.perf_counter_ns()-begin
    phases={k:int(v)for k,v in re.findall(r'(\w+)=(\d+)',p.stderr)};phases['whole']=elapsed
    if tr:samples[name].append(phases)
    subprocess.run([str(J/'checker'),str(inp),str(out),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  row=dict(input=fn,samples_ns=samples,median_ms={x:{k:statistics.median(a[k]for a in ss)/1e6 for k in ss[0]}for x,ss in samples.items()});r['results'].append(row);print(row['input'],row['median_ms'],flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
r['harness_sha256']=hashlib.sha256(Path(__file__).read_bytes()).hexdigest();(W/'profile.json').write_text(json.dumps(r,indent=2)+'\n')
