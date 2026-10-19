from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess
W=Path('/tmp/fps-ifma-leaf-probe');r=json.loads((W/'metadata.json').read_text());r.update({'trials':21,'warmups':1,'shuffle_seed':2026101901,'loops_per_sample':1048576,'units':'ns per leaf','cases':{},'execution_order':[]});lock=open('/tmp/cp-fps-tuning.lock','w');print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('acquired EX',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(2026101901)
modes=['base','ifma_inline_g1','ifma_inline_g2','ifma_inline','base_call','ifma_call','dispatch','forced_fallback']
for count in [1,128,4096]:
 samples={v:[]for v in modes};hashes=set()
 for trial in range(22):
  order=list(modes);rng.shuffle(order);r['execution_order'].append([count,trial,order])
  for mode in order:
   ns,h=subprocess.check_output([str(W/'micro'),mode,str(count),'1048576'],text=True).split();hashes.add(h)
   if trial:samples[mode].append(float(ns))
 assert len(hashes)==1,(count,hashes)
 r['cases'][str(count)]={'output_hash':next(iter(hashes)),'items':count,'variants':{v:{'median_ns':statistics.median(s),'samples_ns':s}for v,s in samples.items()}};(W/'results.json').write_text(json.dumps(r,indent=2)+'\n');print(count,{v:row['median_ns']for v,row in r['cases'][str(count)]['variants'].items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
