from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-avx512-top');D=W/'official_compare';F=Path('/tmp/library-checker-problems/polynomial/sqrt_of_formal_power_series');P=F.name;V=['base','late2','reference'];sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
previous=json.loads((W/'whole_late2.json').read_text());r={k:previous[k]for k in ['baseline','flags','compiler','hardware','headers_before']};r.update(cpu=2,trials=9,seed=20261029,sources={},results=[])
lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_SH)
for v in V:
 key=P+'_'+v;row=previous['sources'][key];assert row['binary_sha256']==sha(D/key);r['sources'][key]=row
fcntl.flock(lock,fcntl.LOCK_UN);print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('EX acquired',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(r['seed'])
inp=F/'in/max_random_01.in';ans=F/'out/max_random_01.out';samples={v:[]for v in V}
for trial in range(r['trials']+1):
 order=V.copy();rng.shuffle(order)
 for v in order:
  out=D/(P+'_max_random_01_'+v+'.out')
  with inp.open('rb')as fi,out.open('wb')as fo:
   start=time.perf_counter_ns();subprocess.run([str(D/(P+'_'+v))],stdin=fi,stdout=fo,check=True);ms=(time.perf_counter_ns()-start)/1e6
  subprocess.run([str(F/'checker'),str(inp),str(out),str(ans)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  if trial:samples[v].append(ms)
r['results'].append(dict(problem=P,case='max_random_01',input_sha256=sha(inp),answer_sha256=sha(ans),checker_sha256=sha(F/'checker'),variants={v:dict(median_ms=statistics.median(s),samples_ms=s,output_sha256=sha(D/(P+'_max_random_01_'+v+'.out')))for v,s in samples.items()}));(W/'whole_late2_sqrt01.json').write_text(json.dumps(r,indent=2)+'\n');print({v:statistics.median(s)for v,s in samples.items()},flush=True);fcntl.flock(lock,fcntl.LOCK_UN)
