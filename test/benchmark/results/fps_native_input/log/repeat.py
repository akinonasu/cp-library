from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-native-source');J=Path('/tmp/library-checker-problems/polynomial/log_of_formal_power_series');inp=J/'in/max_random_00.in';answer=J/'out/max_random_00.out';lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261020);samples={v:[]for v in ['before','after_shared']}
for trial in range(100):
 order=list(samples);rng.shuffle(order)
 for v in order:
  output=W/'repeat.out'
  with inp.open('rb')as fin,output.open('wb')as fout:
   start=time.perf_counter_ns();subprocess.run([str(W/(v+'_whole'))],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
  subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
  if trial:samples[v].append(elapsed)
 if trial in [0,24,49,74,99]:print('trial',trial,flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
r={'cpu':2,'trials':99,'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'variants':{v:{'samples_ms':s,'median_ms':statistics.median(s),'binary_sha256':hashlib.sha256((W/(v+'_whole')).read_bytes()).hexdigest()}for v,s in samples.items()}}
(W/'repeat.json').write_text(json.dumps(r,indent=2)+'\n');print({v:x['median_ms']for v,x in r['variants'].items()},flush=True)
