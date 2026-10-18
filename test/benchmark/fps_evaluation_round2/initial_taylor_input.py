from pathlib import Path
import subprocess,fcntl,shutil,json,hashlib,random,statistics,time,os
ROOT=Path('/workspace/cp-library');BASE=Path('/tmp/cp-fps-round2-base');W=Path('/tmp/fps-eval-round2/screen');W.mkdir(parents=True,exist_ok=True)
os.sched_setaffinity(0,{2});flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];result={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','cpu':2,'flags':flags,'trials':21,'sources':{},'results':[]}
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
with open('/tmp/cp-fps-tuning.lock','w')as lock:
 for variant in ['before','fused']:
  dest=W/variant
  if dest.exists():shutil.rmtree(dest)
  shutil.copytree(BASE,dest)
  if variant=='fused':shutil.copy2(ROOT/'components/fps_taylor_blocked.hpp',dest/'components/fps_taylor_blocked.hpp')
  result['sources'][variant]={str(p.relative_to(dest)):sha(p)for p in dest.rglob('*.hpp')}
  fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run(['g++',*flags,'-I'+str(dest),str(dest/'test/fps_taylor_shift.test.cpp'),'-o',str(W/(variant+'.out'))],check=True);fcntl.flock(lock,fcntl.LOCK_UN)
  result['sources'][variant+'_binary']=sha(W/(variant+'.out'));print('built',variant,flush=True)
 ref=Path('/tmp/fps-reference/polynomial_taylor_shift/409534.cpp')
 fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run(['g++',*flags,str(ref),'-o',str(W/'reference.out')],check=True);fcntl.flock(lock,fcntl.LOCK_UN)
 result['sources']['reference']={'source':sha(ref),'binary':sha(W/'reference.out')};print('built reference',flush=True)
 J=Path('/tmp/library-checker-problems/polynomial/polynomial_taylor_shift')
 fcntl.flock(lock,fcntl.LOCK_SH)
 for inp in sorted((J/'in').glob('*.in')):
  output=W/'check.out'
  with inp.open('rb')as fin,output.open('wb')as fout:subprocess.run([str(W/'fused.out')],stdin=fin,stdout=fout,check=True)
  subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 fcntl.flock(lock,fcntl.LOCK_UN);print('checked all38',flush=True)
 fcntl.flock(lock,fcntl.LOCK_EX);rng=random.Random(20261019)
 for name in ['max_random_00.in','max_random_01.in']:
  inp=J/'in'/name;samples={v:[]for v in ['before','fused','reference']}
  for trial in range(22):
   order=list(samples);rng.shuffle(order)
   for v in order:
    output=W/'check.out'
    with inp.open('rb')as fin,output.open('wb')as fout:
     t=time.perf_counter_ns();subprocess.run([str(W/(v+'.out'))],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-t)/1e6
    if trial:samples[v].append(elapsed)
    subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  row={'input':name,'input_sha256':sha(inp),'samples_ms':samples,'median_ms':{v:statistics.median(x)for v,x in samples.items()}}
  result['results'].append(row);print(name,row['median_ms'],flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
result['harness_sha256']=sha(Path(__file__));(W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
