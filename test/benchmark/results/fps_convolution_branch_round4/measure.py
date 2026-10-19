import fcntl,hashlib,json,os,random,statistics,subprocess,time,shlex
from pathlib import Path
W=Path(__file__).resolve().parent;S=W/'source';P=Path('/workspace/cp-fps-round4/final-convolution-constant');O=Path('/tmp/library-checker-problems/convolution/convolution_mod');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
F=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']; B={v:P/v for v in ['before','current','reference']};B['hint']=W/'hint';R={'script_sha256':sha(Path(__file__)),'flags':F,'compiler':subprocess.check_output(['g++','--version'],text=True),'compiler_sha256':sha(Path('/usr/bin/g++').resolve()),'headers':{str(p.relative_to(S)):sha(p) for p in S.rglob('*') if p.is_file()},'trials':21,'rows':[]}
with open('/tmp/cp-fps-tuning.lock','a') as l:
 fcntl.flock(l,fcntl.LOCK_SH)
 cmd=['g++',*F,'-MD','-MF',str(W/'hint.d'),'-I'+str(S),str(S/'test/convolution.test.cpp'),'-o',str(B['hint'])];R['command']=cmd;subprocess.run(cmd,check=True)
 deps=shlex.split((W/'hint.d').read_text().replace('\\\n',' ').partition(':')[2]);R['dependencies_before']={p:sha(Path(p)) for p in deps};R['binaries_before']={v:sha(p) for v,p in B.items()}
 fcntl.flock(l,fcntl.LOCK_UN);fcntl.flock(l,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261019)
 for name in ['max_random_00','max_random_01','all_same_00','all_same_01']:
  inp=O/'in'/(name+'.in');ans=O/'out'/(name+'.out');a={v:[] for v in B};runs=[]
  for rep in range(22):
   order=list(B);rng.shuffle(order);run={'order':order,'samples':{}}
   for v in order:
    out=W/(v+'.actual')
    with inp.open('rb') as si,out.open('wb') as so:
     begin=time.perf_counter_ns();subprocess.run([str(B[v])],stdin=si,stdout=so,check=True);ns=time.perf_counter_ns()-begin
    subprocess.run([str(O/'checker'),str(inp),str(out),str(ans)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    run['samples'][v]={'elapsed_ns':ns,'output_sha256':sha(out),'checker':'accepted'}
    if rep:a[v].append(ns/1e6)
   runs.append(run)
  row={'name':name,'input_sha256':sha(inp),'answer_sha256':sha(ans),'runs':runs,'median_ms':{v:statistics.median(s) for v,s in a.items()},'paired_saving_vs_current':statistics.median([x-y for x,y in zip(a['current'],a['hint'])]),'paired_saving_vs_before':statistics.median([x-y for x,y in zip(a['before'],a['hint'])])};R['rows'].append(row);print(name,row['median_ms'],row['paired_saving_vs_current'],row['paired_saving_vs_before'],flush=True);(W/'result.json').write_text(json.dumps(R,indent=2)+'\n')
 R['dependencies_after']={p:sha(Path(p)) for p in deps};R['binaries_after']={v:sha(p) for v,p in B.items()};assert R['dependencies_after']==R['dependencies_before'] and R['binaries_after']==R['binaries_before'];R['complete']=True;(W/'result.json').write_text(json.dumps(R,indent=2)+'\n')
