from pathlib import Path
import subprocess,fcntl,json,hashlib,random,statistics,time,os
W=Path('/tmp/fps-evaluation-round3');W.mkdir(parents=True,exist_ok=True)
os.sched_setaffinity(0,{2})
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
result=dict(baseline='bd951dce6fdcb3ea9db4fcef342147012a8ab0c0',cpu=2,flags=flags,trials=15,seed=20261023,compiler=subprocess.check_output(['g++','--version'],text=True),hardware=subprocess.check_output(['lscpu'],text=True),sources={},builds={},checks=[],results=[])
binaries={}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 for name in ['base','evex','wide_chain','reference']:
  tree=W/name
  source=Path('/tmp/fps-reference/polynomial_taylor_shift/409534.cpp') if name=='reference' else tree/'test/fps_taylor_shift.test.cpp'
  binary=W/(name+'.out')
  command=['g++',*flags,('-I'+str(tree)),str(source),'-o',str(binary)]
  fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run(command,check=True);fcntl.flock(lock,fcntl.LOCK_UN)
  binaries[name]=binary;result['builds'][name]=dict(command=command,source_sha256=sha(source),binary_sha256=sha(binary))
  if name!='reference':result['sources'][name]={str(p.relative_to(tree)):sha(p)for p in sorted(tree.rglob('*.hpp'))}
  print('built',name,flush=True)
 J=Path('/tmp/library-checker-problems/polynomial/polynomial_taylor_shift')
 for name in ['evex','wide_chain']:
  fcntl.flock(lock,fcntl.LOCK_SH);count=0
  for inp in sorted((J/'in').glob('*.in')):
   output=W/'check.out'
   with inp.open('rb')as src,output.open('wb')as dst:subprocess.run([str(binaries[name])],stdin=src,stdout=dst,check=True)
   subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL);count+=1
  fcntl.flock(lock,fcntl.LOCK_UN);result['checks'].append(dict(variant=name,cases=count));print('checked',name,count,flush=True)
 fcntl.flock(lock,fcntl.LOCK_EX);rng=random.Random(20261023)
 for filename in ['max_random_00.in','max_random_01.in']:
  inp=J/'in'/filename;samples={v:[]for v in binaries};orders=[]
  for trial in range(16):
   order=list(binaries);rng.shuffle(order);orders.append(order)
   for name in order:
    output=W/'check.out'
    with inp.open('rb')as src,output.open('wb')as dst:
     start=time.perf_counter_ns();subprocess.run([str(binaries[name])],stdin=src,stdout=dst,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
    if trial:samples[name].append(elapsed)
    subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/inp.with_suffix('.out').name)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  row=dict(input=filename,input_sha256=sha(inp),orders=orders,samples_ms=samples,median_ms={v:statistics.median(s)for v,s in samples.items()})
  row['paired_median_saving_ms']={v:statistics.median([a-b for a,b in zip(samples['base'],samples[v])])for v in ['evex','wide_chain']}
  row['wins']={v:sum(a>b for a,b in zip(samples['base'],samples[v]))for v in ['evex','wide_chain']}
  result['results'].append(row);print(row['input'],row['median_ms'],row['paired_median_saving_ms'],row['wins'],flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
result['harness_sha256']=sha(Path(__file__));(W/'taylor_screen.json').write_text(json.dumps(result,indent=2)+'\n')
