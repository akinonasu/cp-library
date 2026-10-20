import argparse,fcntl,hashlib,json,os,random,statistics,subprocess,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--trials',type=int,default=5);p.add_argument('--lengths',default='256,512,1024,2048,4096,8192,16384');p.add_argument('--cases',default='small_and_large_00,small_and_large_01');p.add_argument('--name',default='initial');p.add_argument('--seed',type=int,default=20261120);p.add_argument('--dynamic',action='store_true');args=p.parse_args()
w=Path(__file__).resolve().parent
off=Path('/workspace/cp-fps-scratch-round3/library-checker-problems/convolution/convolution_mod')
sha=lambda x:hashlib.sha256(Path(x).read_bytes()).hexdigest()
lengths=[int(x) for x in args.lengths.split(',')]
result={'trials':args.trials,'seed':args.seed,'cpu':2,'flags':json.loads((w/'build.json').read_text())['flags'],'baseline':json.loads((w/'build.json').read_text())['baseline'],'harness_sha256':sha(__file__),'sources':{x:sha(w/x) for x in ['overlap.hpp','driver.cpp','unit.cpp']},'cases':[]}
rng=random.Random(args.seed)
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX)
 for case in args.cases.split(','):
  inp=off/'in'/f'{case}.in';ans=off/'out'/f'{case}.out'
  short=min(map(int,inp.open().readline().split()))
  names=['before','reference',*(['overlap_dynamic4','overlap_dynamic8'] if args.dynamic else [f'overlap_{x}' for x in lengths if x>=short])]
  binaries={name:{'sha256':sha(w/name),'trials_ns':[]} for name in names}
  checker=[];order=[]
  for trial in range(-1,args.trials):
   seq=names[:];rng.shuffle(seq);order.append(seq)
   for name in seq:
    out=w/f'{args.name}_{case}_{name}.out'
    with inp.open('rb') as src,out.open('wb') as dest:
     start=time.perf_counter_ns();subprocess.run(['taskset','-c','2',str(w/name)],stdin=src,stdout=dest,check=True);elapsed=time.perf_counter_ns()-start
    cp=subprocess.run([str(off/'checker'),str(inp),str(out),str(ans)],capture_output=True,text=True)
    checker.append({'trial':trial,'binary':name,'returncode':cp.returncode,'stderr':cp.stderr,'output_sha256':sha(out)})
    if cp.returncode: raise RuntimeError(checker[-1])
    if trial>=0:binaries[name]['trials_ns'].append(elapsed)
  for v in binaries.values():v['median_ns']=statistics.median(v['trials_ns'])
  row={'case':case,'input_sha256':sha(inp),'answer_sha256':sha(ans),'binaries':binaries,'order_including_warmup':order,'checker':checker}
  result['cases'].append(row)
  (w/f'{args.name}.json').write_text(json.dumps(result,indent=2)+'\n')
  print(case,{k:round(v['median_ns']/1e6,3) for k,v in binaries.items()},flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
