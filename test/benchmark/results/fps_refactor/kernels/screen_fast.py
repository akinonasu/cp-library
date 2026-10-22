from pathlib import Path
import subprocess,fcntl,os,json,random,statistics,hashlib,time
w=Path('/workspace/cp-fps-refactor-kernels-study')
variants=['baseline','all_top','radix_fast','shared_product']
cases=[(r['op'],r['n']) for r in json.loads((w/'correctness.json').read_text())]
cases += [(op,65536) for op in ['multiply','square','inv','log','exp','pow','sqrt','div_series','divmod','compose','compositional_inverse','power_projection','bostan_mori','linear_recurrence','multipoint_eval','interpolate','taylor_shift']]
cases += [('multiply',262144),('multiply',524288),('compose',131072),('multipoint_eval',131072)]
cases=list(dict.fromkeys(cases))
os.sched_setaffinity(0,{2})
report={'trials':5,'seed':20261021,'cpu':2,'variants':variants,'binary_sha256':{v:hashlib.sha256((w/(v+'.bin')).read_bytes()).hexdigest() for v in variants},'rows':[],'complete':False}
rng=random.Random(20261021)
with open('/tmp/cp-fps-tuning.lock','a') as one,open('/tmp/cp-wm-opt.lock','a') as two:
 print('waiting for locks',flush=True)
 fcntl.flock(one,fcntl.LOCK_EX);fcntl.flock(two,fcntl.LOCK_EX)
 print('timing started',flush=True)
 for op,n in cases:
  repeat=3 if n>=65536 else 16 if n>=1024 else 32
  values={v:[] for v in variants}; checks=set()
  for trial in range(5):
   order=variants.copy();rng.shuffle(order)
   for v in order:
    row=json.loads(subprocess.check_output([str(w/(v+'.bin')),op,str(n),str(repeat)],text=True,timeout=120))
    values[v].append(row['ms']);checks.add((row['checksum'],row['size']))
  assert len(checks)==1,(op,n,checks)
  med={v:statistics.median(t) for v,t in values.items()}
  result={'op':op,'n':n,'repeat':repeat,'median_ms':med,'slowdown':{v:med[v]/med['baseline'] for v in variants},'samples':values}
  report['rows'].append(result)
  (w/'screen_fast.json').write_text(json.dumps(report,indent=2)+'\n')
  print(op,n,{v:round(result['slowdown'][v],3) for v in variants},flush=True)
report['complete']=True
(w/'screen_fast.json').write_text(json.dumps(report,indent=2)+'\n')
