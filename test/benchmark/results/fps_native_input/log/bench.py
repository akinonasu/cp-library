from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-native-source');J=Path('/tmp/library-checker-problems/polynomial/log_of_formal_power_series');lock=open('/tmp/cp-fps-tuning.lock','w')
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];result={'flags':flags,'cpu':2,'trials':21,'kernel':{},'whole':{},'sources':{},'baseline':'4765ceb'}
fcntl.flock(lock,fcntl.LOCK_SH)
for v in ['before','after']:
 root=W/v
 for kind,source in [('kernel',W/'kernel.cpp'),('whole',root/'test/fps_log.test.cpp')]:
  binary=W/(v+'_'+kind);cmd=['g++',*flags,'-I',str(root),str(source),'-o',str(binary)];subprocess.run(cmd,check=True)
  result['sources'][v+'_'+kind]={'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'command':cmd};print('built',v,kind,flush=True)
 result['sources'][v+'_headers']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for folder in ['components','fps','math','convolution'] for p in (root/folder).rglob('*.hpp')}
ref=Path('/tmp/fps-reference/log_of_formal_power_series/409452.cpp');binary=W/'reference_whole';subprocess.run(['g++',*flags,str(ref),'-o',str(binary)],check=True)
result['sources']['reference_whole']={'source_sha256':hashlib.sha256(ref.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest()};fcntl.flock(lock,fcntl.LOCK_UN)
fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261019)
for n in [32768,131073,500000,524288]:
 samples={v:[] for v in ['before','after']};hashes=set()
 for trial in range(22):
  order=list(samples);rng.shuffle(order)
  for v in order:
   text=subprocess.check_output([str(W/(v+'_kernel')),str(n)],text=True);ms,h=text.split();hashes.add(h)
   if trial:samples[v].append(float(ms))
 assert len(hashes)==1,(n,hashes)
 row={v:{'median_ms':statistics.median(s),'samples_ms':s} for v,s in samples.items()};result['kernel'][str(n)]=row;print('kernel',n,{v:x['median_ms']for v,x in row.items()},flush=True)
for name in ['max_random_00','max_random_01']:
 samples={v:[]for v in ['before','after','reference']};inp=J/'in'/(name+'.in');answer=J/'out'/(name+'.out')
 for trial in range(22):
  order=list(samples);rng.shuffle(order)
  for v in order:
   output=W/'output.txt'
   with inp.open('rb')as fin,output.open('wb')as fout:
    t=time.perf_counter_ns();subprocess.run([str(W/(v+'_whole'))],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-t)/1e6
   subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
   if trial:samples[v].append(elapsed)
 row={v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()};result['whole'][name]={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'variants':row};print(name,{v:x['median_ms']for v,x in row.items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN);(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
