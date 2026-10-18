from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-round2-conv-pair');B=Path('/tmp/cp-fps-round2-base');C=W/'candidate';R=Path('/tmp/fps-reference/convolution_mod/409465.cpp');J=Path('/tmp/library-checker-problems/convolution/convolution_mod')
if not J.exists():J=Path('/tmp/library-checker-problems/polynomial/convolution_mod')
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
res={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','flags':flags,'cpu':2,'trials':21,'sources':{},'kernel':{},'whole':{}}
lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH)
for v,tree in [('base',B),('candidate',C)]:
 for kind,src in [('kernel',B/'test/benchmark/fps_tuning_basic.cpp'),('whole',tree/'test/convolution.test.cpp')]:
  dst=W/(v+'_'+kind);cmd=['g++',*flags,'-I',str(tree),str(src),'-o',str(dst)];subprocess.run(cmd,check=True);res['sources'][v+'_'+kind]={'command':cmd,'source_sha256':sha(src),'binary_sha256':sha(dst)};print('built',v,kind,flush=True)
 res['sources'][v+'_header_sha256']=sha(tree/'components/fps_convolution.hpp')
dst=W/'reference_whole';cmd=['g++',*flags,str(R),'-o',str(dst)];subprocess.run(cmd,check=True);res['sources']['reference_whole']={'command':cmd,'source_sha256':sha(R),'binary_sha256':sha(dst)};print('built reference',flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261021)
for n in [8192,131072,500000,524288]:
 samples={v:[]for v in ['base','candidate']};hashes=set()
 for trial in range(22):
  order=list(samples);rng.shuffle(order)
  for v in order:
   elapsed,h=subprocess.check_output([str(W/(v+'_kernel')),'multiply',str(n)],text=True).split();hashes.add(h)
   if trial:samples[v].append(float(elapsed)*1000)
 assert len(hashes)==1
 res['kernel'][str(n)]={v:{'samples_ms':s,'median_ms':statistics.median(s)}for v,s in samples.items()};print('kernel',n,{v:statistics.median(s)for v,s in samples.items()},flush=True)
for name in ['max_random_00','max_random_01']:
 inp=J/'in'/(name+'.in');ans=J/'out'/(name+'.out');samples={v:[]for v in ['base','candidate','reference']}
 for trial in range(22):
  order=list(samples);rng.shuffle(order)
  for v in order:
   out=W/(v+'.out')
   with inp.open('rb')as fin,out.open('wb')as fout:
    start=time.perf_counter_ns();subprocess.run([str(W/(v+'_whole'))],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
   subprocess.run([str(J/'checker'),str(inp),str(out),str(ans)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   if trial:samples[v].append(elapsed)
 res['whole'][name]={'input_sha256':sha(inp),'answer_sha256':sha(ans),'checker_sha256':sha(J/'checker'),'variants':{v:{'samples_ms':s,'median_ms':statistics.median(s)}for v,s in samples.items()}};print('whole',name,{v:statistics.median(s)for v,s in samples.items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN);(W/'results.json').write_text(json.dumps(res,indent=2)+'\n')
