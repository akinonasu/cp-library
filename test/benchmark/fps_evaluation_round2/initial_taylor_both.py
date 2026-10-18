from pathlib import Path
import subprocess,fcntl,shutil,json,hashlib,random,statistics,time,os
ROOT=Path('/workspace/cp-library');BASE=Path('/tmp/cp-fps-round2-base');W=Path('/tmp/fps-eval-round2/both');W.mkdir(parents=True,exist_ok=True)
os.sched_setaffinity(0,{2});flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];result={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','cpu':2,'flags':flags,'trials':21,'sources':{},'results':[]};sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
with open('/tmp/cp-fps-tuning.lock','w')as lock:
 C=W/'candidate'
 if C.exists():shutil.rmtree(C)
 shutil.copytree(BASE,C);shutil.copy2(ROOT/'components/fps_taylor_blocked.hpp',C/'components/fps_taylor_blocked.hpp')
 fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run(['g++',*flags,'-I'+str(C),str(C/'test/fps_taylor_shift.test.cpp'),'-o',str(W/'both.out')],check=True);fcntl.flock(lock,fcntl.LOCK_UN);print('built both',flush=True)
 bins={'before':Path('/tmp/fps-eval-round2/screen/before.out'),'input':Path('/tmp/fps-eval-round2/screen/fused.out'),'both':W/'both.out','reference':Path('/tmp/fps-eval-round2/screen/reference.out')}
 result['sources']['headers']={str(p.relative_to(C)):sha(p)for p in C.rglob('*.hpp')};result['sources']['binaries']={v:sha(p)for v,p in bins.items()}
 J=Path('/tmp/library-checker-problems/polynomial/polynomial_taylor_shift');fcntl.flock(lock,fcntl.LOCK_SH)
 for inp in sorted((J/'in').glob('*.in')):
  output=W/'check.out'
  with inp.open('rb')as fin,output.open('wb')as fout:subprocess.run([str(bins['both'])],stdin=fin,stdout=fout,check=True)
  subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 fcntl.flock(lock,fcntl.LOCK_UN);print('checked all38',flush=True)
 fcntl.flock(lock,fcntl.LOCK_EX);rng=random.Random(20261019)
 for name in ['max_random_00.in','max_random_01.in']:
  inp=J/'in'/name;samples={v:[]for v in bins}
  for trial in range(22):
   order=list(samples);rng.shuffle(order)
   for v in order:
    output=W/'check.out'
    with inp.open('rb')as fin,output.open('wb')as fout:
     t=time.perf_counter_ns();subprocess.run([str(bins[v])],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-t)/1e6
    if trial:samples[v].append(elapsed)
    subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  row={'input':name,'input_sha256':sha(inp),'samples_ms':samples,'median_ms':{v:statistics.median(x)for v,x in samples.items()}}
  result['results'].append(row);print(name,row['median_ms'],flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
result['harness_sha256']=sha(Path(__file__));(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
