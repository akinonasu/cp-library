from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-pow-native-probe');J=Path('/tmp/library-checker-problems/polynomial/pow_of_formal_power_series');lock=open('/tmp/cp-fps-tuning.lock','w')
result=json.loads((W/'metadata.json').read_text());result.update({'kernel':{},'whole':{},'execution_order':[],'shuffle_seed':20261019,'warmups':1})
def save(): (W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('acquired EX',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(20261019)
variants=['baseline','candidate','reference']
for n in [262144,262145,393217,500000]:
 samples={v:[] for v in variants};hashes=set()
 for trial in range(22):
  order=list(samples);rng.shuffle(order);result['execution_order'].append(['kernel',n,trial,order])
  for v in order:
   output=subprocess.check_output([str(W/(v+'_kernel')),str(n)],text=True);ms,h=output.split();hashes.add(h)
   if trial:samples[v].append(float(ms))
 assert len(hashes)==1,(n,hashes)
 row={v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()};result['kernel'][str(n)]={'variants':row,'output_hash':next(iter(hashes))};save();print('kernel',n,{v:x['median_ms']for v,x in row.items()},flush=True)
for name in ['max_random_00','max_random_01']:
 samples={v:[] for v in variants};inp=J/'in'/(name+'.in');answer=J/'out'/(name+'.out');hashes={v:set()for v in variants}
 for trial in range(22):
  order=list(samples);rng.shuffle(order);result['execution_order'].append(['whole',name,trial,order])
  for v in order:
   output=W/(v+'_output.txt')
   with inp.open('rb')as fin,output.open('wb')as fout:
    start=time.perf_counter_ns();subprocess.run([str(W/(v+'_whole'))],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
   subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
   hashes[v].add(hashlib.sha256(output.read_bytes()).hexdigest())
   if trial:samples[v].append(elapsed)
 for v in variants:assert len(hashes[v])==1
 row={v:{'median_ms':statistics.median(s),'samples_ms':s,'output_sha256':next(iter(hashes[v]))}for v,s in samples.items()};result['whole'][name]={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'answer_sha256':hashlib.sha256(answer.read_bytes()).hexdigest(),'checker_sha256':hashlib.sha256((J/'checker').read_bytes()).hexdigest(),'variants':row};save();print('whole',name,{v:x['median_ms']for v,x in row.items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
