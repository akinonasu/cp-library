from pathlib import Path
import subprocess,fcntl,shutil,json,hashlib,random,statistics,time,os
B=Path('/tmp/cp-fps-round2-base');W=Path('/tmp/fps-eval-round2/vectors');W.mkdir(parents=True,exist_ok=True);os.sched_setaffinity(0,{2});flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();result={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','cpu':2,'flags':flags,'trials':15,'sources':{},'results':[]}
with open('/tmp/cp-fps-tuning.lock','w')as lock:
 bins={'vectors4':Path('/tmp/fps-eval-round2/screen/before.out'),'reference':Path('/tmp/fps-eval-round2/screen/reference.out')}
 for count in [1,2]:
  C=W/('vectors'+str(count))
  if C.exists():shutil.rmtree(C)
  shutil.copytree(B,C);p=C/'components/fps_taylor_blocked.hpp';s=p.read_text().replace('kVectors = 4, kLanes = 8 * kVectors','kVectors = '+str(count)+', kLanes = 8 * kVectors');p.write_text(s)
  binary=W/('vectors'+str(count)+'.out');fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run(['g++',*flags,'-I'+str(C),str(C/'test/fps_taylor_shift.test.cpp'),'-o',str(binary)],check=True);fcntl.flock(lock,fcntl.LOCK_UN);bins['vectors'+str(count)]=binary;result['sources']['vectors'+str(count)]={'header':sha(p),'binary':sha(binary)};print('built',count,flush=True)
 fcntl.flock(lock,fcntl.LOCK_EX);rng=random.Random(20261020);J=Path('/tmp/library-checker-problems/polynomial/polynomial_taylor_shift')
 for name in ['max_random_00.in','max_random_01.in']:
  inp=J/'in'/name;samples={v:[]for v in bins}
  for trial in range(16):
   order=list(samples);rng.shuffle(order)
   for v in order:
    output=W/'check.out'
    with inp.open('rb')as fin,output.open('wb')as fout:
     start=time.perf_counter_ns();subprocess.run([str(bins[v])],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
    if trial:samples[v].append(elapsed)
    subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  row={'input':name,'input_sha256':sha(inp),'samples_ms':samples,'median_ms':{v:statistics.median(x)for v,x in samples.items()}};result['results'].append(row);print(name,row['median_ms'],flush=True)
 result['harness_sha256']=sha(Path(__file__));(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
