from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
w=Path(__file__).resolve().parent;off=Path('/tmp/library-checker-problems/convolution/convolution_mod');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
bins={name:w/(name+'.bin') for name in ['before','candidate','reference']}
meta={'baseline':'bd29f70c70e24e27770b9e9a3777b6cf85664b63 plus frozen convolution/unbalanced changes','trials':9,'seed':20261124,'cpu':2,'commands':json.loads((w/'build.json').read_text())['commands'],'flags':json.loads((w/'build.json').read_text())['flags'],'harness_sha256':sha(Path(__file__)),'source_snapshot_sha256':sha(w/'snapshot.json'),'source_sha256':{str(p.relative_to(w)):sha(p) for p in [w/'candidate/components/fps_convolution_constant.hpp',w/'candidate/components/fps_convolution.hpp',w/'unit.cpp']},'binary_sha256':{name:sha(p) for name,p in bins.items()},'checker_sha256':sha(off/'checker'),'cases':[]}
rng=random.Random(meta['seed'])
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2})
 for case in ['all_same_00','all_same_01','max_random_00','max_random_01']:
  inp=off/'in'/(case+'.in');ans=off/'out'/(case+'.out');samples={v:[] for v in bins};orders=[];checks=[]
  for rep in range(-1,meta['trials']):
   order=list(bins);rng.shuffle(order);orders.append(order)
   for name in order:
    out=w/(case+'_'+name+'.out')
    with inp.open('rb') as src,out.open('wb') as dest:
     start=time.perf_counter_ns();subprocess.run([str(bins[name])],stdin=src,stdout=dest,check=True);elapsed=time.perf_counter_ns()-start
    check=subprocess.run([str(off/'checker'),str(inp),str(out),str(ans)],capture_output=True,text=True)
    checks.append({'rep':rep,'variant':name,'returncode':check.returncode,'stderr':check.stderr,'output_sha256':sha(out)})
    if check.returncode:raise RuntimeError(checks[-1])
    if rep>=0:samples[name].append(elapsed)
  delta=[a-b for a,b in zip(samples['before'],samples['candidate'])]
  row={'case':case,'input_sha256':sha(inp),'answer_sha256':sha(ans),'samples_ns':samples,'orders_including_warmup':orders,'checks':checks,'median_ns':{name:statistics.median(v) for name,v in samples.items()},'paired_saving_ns':statistics.median(delta),'wins':sum(d>0 for d in delta)};meta['cases'].append(row)
  (w/'screen9.json').write_text(json.dumps(meta,indent=2)+'\n');print(case,row['median_ns'],'paired',row['paired_saving_ns'],'wins',row['wins'],flush=True)
 assert all(sha(p)==meta['binary_sha256'][name] for name,p in bins.items())
 fcntl.flock(lock,fcntl.LOCK_UN)
