from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-native-source');J=Path('/tmp/library-checker-problems/polynomial/log_of_formal_power_series');lock=open('/tmp/cp-fps-tuning.lock','w')
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
flags[4]='-ftrivial-auto-var-init=zero'
result={'flags':flags,'cpu':2,'trials':21,'kernel':{},'whole':{},'sources':{},'baseline':'4765ceb'}
fcntl.flock(lock,fcntl.LOCK_SH)
for kind,source in [('kernel',W/'kernel.cpp'),('whole',W/'after_shared/test/fps_log.test.cpp')]:
 binary=W/('after_shared_'+kind);subprocess.run(['g++',*flags,'-I',str(W/'after_shared'),str(source),'-o',str(binary)],check=True);print('built',kind,flush=True)
fcntl.flock(lock,fcntl.LOCK_UN);fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261019)
for n in [131073,262144,262145,393217,500000,524288]:
 samples={v:[]for v in ['before','after_shared']};hashes=set()
 for trial in range(22):
  order=list(samples);rng.shuffle(order)
  for v in order:
   ms,h=subprocess.check_output([str(W/(v+'_kernel')),str(n)],text=True).split();hashes.add(h)
   if trial:samples[v].append(float(ms))
 assert len(hashes)==1
 row={v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()};result['kernel'][str(n)]=row;print('kernel',n,{v:x['median_ms']for v,x in row.items()},flush=True)
# Official N=500000 plus generated boundary inputs from the same official coefficient stream.
original=(J/'in/max_random_00.in').read_text().split()
for n in [262144,262145,393217,500000]:
 inp=W/('input_'+str(n)+'.in');inp.write_text(str(n)+'\n'+' '.join(original[1:n+1])+'\n');answer=W/('answer_'+str(n)+'.out')
 with inp.open('rb')as fin,answer.open('wb')as fout:subprocess.run([str(W/'reference_whole')],stdin=fin,stdout=fout,check=True)
 samples={v:[]for v in ['before','after_shared']}
 for trial in range(22):
  order=list(samples);rng.shuffle(order)
  for v in order:
   output=W/'output.txt'
   with inp.open('rb')as fin,output.open('wb')as fout:
    t=time.perf_counter_ns();subprocess.run([str(W/(v+'_whole'))],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-t)/1e6
   subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
   if trial:samples[v].append(elapsed)
 row={v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()};result['whole'][str(n)]={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'variants':row};print('whole',n,{v:x['median_ms']for v,x in row.items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
for v in ['before','after_shared']:
 result['sources'][v+'_headers']={str(p.relative_to(W/v)):hashlib.sha256(p.read_bytes()).hexdigest()for folder in ['components','fps','math','convolution']for p in (W/v/folder).rglob('*.hpp')}
 for k in ['kernel','whole']:result['sources'][v+'_'+k]=hashlib.sha256((W/(v+'_'+k)).read_bytes()).hexdigest()
(W/'shared.json').write_text(json.dumps(result,indent=2)+'\n')
