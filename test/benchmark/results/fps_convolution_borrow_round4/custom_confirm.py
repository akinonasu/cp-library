import fcntl,hashlib,json,os,random,statistics,subprocess,time
from pathlib import Path
W=Path('/workspace/cp-fps-round4/root'); D=W/'custom_confirm'; D.mkdir(exist_ok=True); I=W/'custom'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
bins={v:W/'fallback_first'/v for v in ['before','borrowed','split','reference']}
checker=Path('/tmp/library-checker-problems/convolution/convolution_mod/checker')
inputs=[]
meta={'script_sha256':sha(Path(__file__)),'baseline':'7521ea8703a362aaf3ccd370188fccab86050848','checker_sha256':sha(checker),'binary_sha256':{v:sha(p) for v,p in bins.items()},'trials':21,'rows':[]}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 for n,m in [(65536,65536),(262144,262144),(300000,220000)]:
  p=I/f'{n}_{m}.in'; a=I/f'{n}_{m}.out'
  if not p.exists():
   rng=random.Random(617+n)
   with p.open('w') as f:
    f.write(f'{n} {m}\n');f.write(' '.join(str(rng.randrange(998244353)) for _ in range(n))+'\n');f.write(' '.join(str(rng.randrange(998244353)) for _ in range(m))+'\n')
  if not a.exists():
   with p.open('rb') as si,a.open('wb') as so:subprocess.run([str(bins['reference'])],stdin=si,stdout=so,check=True)
  inputs.append((p,a))
 for name in ['max_random_00','max_random_01']:
  d=Path('/tmp/library-checker-problems/convolution/convolution_mod');inputs.append((d/'in'/(name+'.in'),d/'out'/(name+'.out')))
 fcntl.flock(lock,fcntl.LOCK_UN);fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261027)
 for p,a in inputs:
  samples={v:[] for v in bins};orders=[]
  for rep in range(meta['trials']+1):
   order=list(bins);rng.shuffle(order);orders.append(order)
   for v in order:
    o=D/(p.stem+'_'+v+'.actual')
    with p.open('rb') as si,o.open('wb') as so:
     begin=time.perf_counter_ns();subprocess.run([str(bins[v])],stdin=si,stdout=so,check=True);elapsed=(time.perf_counter_ns()-begin)/1e6
    subprocess.run([str(checker),str(p),str(o),str(a)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    if rep:samples[v].append(elapsed)
  delta=[x-y for x,y in zip(samples['before'],samples['borrowed'])]
  r={'input':str(p),'input_sha256':sha(p),'answer_sha256':sha(a),'orders':orders,'samples_ms':samples,'median_ms':{v:statistics.median(s) for v,s in samples.items()},'paired_saving_ms':statistics.median(delta),'wins':sum(x>0 for x in delta)}
  r['paired_saving_by_variant']={v:statistics.median([x-y for x,y in zip(samples['before'],samples[v])]) for v in ['borrowed','split']};r['wins_by_variant']={v:sum(x>y for x,y in zip(samples['before'],samples[v])) for v in ['borrowed','split']};meta['rows'].append(r);print(p.stem,r['median_ms'],r['paired_saving_by_variant'],r['wins_by_variant'],flush=True)
  (W/'custom_confirm.json').write_text(json.dumps(meta,indent=2)+'\n')
 assert all(sha(p)==meta['binary_sha256'][v] for v,p in bins.items())
