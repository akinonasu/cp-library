from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import fcntl,hashlib,json,os,random,statistics,subprocess,time

W=Path('/tmp/fps-avx512-top');B=Path('/tmp/fps-avx512-top-base');O=Path('/tmp/library-checker-problems');P='log_of_formal_power_series';F=O/'polynomial'/P
variants={'base':B,'wide':W/'wide'}
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
r={'baseline':'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0','flags':flags,'cpu':2,'trials':21,'seed':20261025,'problem':P,'sources':{},'results':[]}
lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH)
jobs=[]
for v,t in variants.items():
 src=next(p for p in sorted((t/'test').glob('*.test.cpp')) if '"https://judge.yosupo.jp/problem/'+P+'"' in p.read_text());out=W/(v+'_whole');jobs.append((v,src,out,['g++',*flags,'-I',str(t),str(src),'-o',str(out)]))
src=Path('/tmp/fps-reference')/P/'409452.cpp';out=W/'reference_whole';jobs.append(('reference',src,out,['g++',*flags,str(src),'-o',str(out)]))
def build(job):
 v,src,out,cmd=job;subprocess.run(cmd,check=True);return v,{'command':cmd,'source_sha256':sha(src),'binary_sha256':sha(out),'header_sha256':None if v=='reference' else sha(variants[v]/'components/fps_blocked.hpp')}
with ThreadPoolExecutor(max_workers=2) as pool:
 for v,d in pool.map(build,jobs):r['sources'][v]=d;print('built',v,flush=True)
fcntl.flock(lock,fcntl.LOCK_UN);print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('EX acquired',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(r['seed'])
for case in ['max_random_00','max_random_01']:
 inp=F/'in'/(case+'.in');ans=F/'out'/(case+'.out');samples={v:[]for v in ['base','wide','reference']}
 for trial in range(r['trials']+1):
  order=list(samples);rng.shuffle(order)
  for v in order:
   out=W/(v+'_whole_'+case+'.out')
   with inp.open('rb')as fin,out.open('wb')as fout:
    start=time.perf_counter_ns();subprocess.run([str(W/(v+'_whole'))],stdin=fin,stdout=fout,check=True);ms=(time.perf_counter_ns()-start)/1e6
   subprocess.run([str(F/'checker'),str(inp),str(out),str(ans)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   if trial:samples[v].append(ms)
 row={'case':case,'input_sha256':sha(inp),'answer_sha256':sha(ans),'checker_sha256':sha(F/'checker'),'variants':{v:{'median_ms':statistics.median(s),'samples_ms':s,'output_sha256':sha(W/(v+'_whole_'+case+'.out'))}for v,s in samples.items()}};r['results'].append(row);(W/'whole_wide.json').write_text(json.dumps(r,indent=2)+'\n');print(case,{v:d['median_ms']for v,d in row['variants'].items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
