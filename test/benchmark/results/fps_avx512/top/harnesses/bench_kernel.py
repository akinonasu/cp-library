from pathlib import Path
import concurrent.futures,fcntl,hashlib,json,os,random,statistics,subprocess
W=Path('/tmp/fps-avx512-top');B=Path('/tmp/fps-avx512-top-base');flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];trees={'base':B,'evex':W/'evex','reordered':W/'reordered'};r={'baseline':'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0','flags':flags,'cpu':2,'trials':21,'sources':{},'kernel':{}}
lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH)
def build(v):
 src=B/'test/benchmark/fps_tuning_basic.cpp';out=W/(v+'_kernel');cmd=['g++',*flags,'-I',str(trees[v]),str(src),'-o',str(out)];subprocess.run(cmd,check=True);return v,{'command':cmd,'binary_sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'header_sha256':hashlib.sha256((trees[v]/'components/fps_blocked.hpp').read_bytes()).hexdigest()}
with concurrent.futures.ThreadPoolExecutor(max_workers=3)as pool:
 for v,x in pool.map(build,trees):r['sources'][v]=x;print('built',v,flush=True)
fcntl.flock(lock,fcntl.LOCK_UN);print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('EX acquired',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(20261023)
for op in ['inv','sqrt','log','exp']:
 r['kernel'][op]={}
 for n in [262144,500000]:
  samples={v:[]for v in trees};hashes=set()
  for trial in range(22):
   order=list(trees);rng.shuffle(order)
   for v in order:
    sec,h=subprocess.check_output([str(W/(v+'_kernel')),op,str(n)],text=True).split();hashes.add(h)
    if trial:samples[v].append(float(sec)*1000)
  assert len(hashes)==1,(op,n,hashes)
  row={v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()};r['kernel'][op][str(n)]=row;print(op,n,{v:x['median_ms']for v,x in row.items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN);(W/'kernel.json').write_text(json.dumps(r,indent=2)+'\n')
