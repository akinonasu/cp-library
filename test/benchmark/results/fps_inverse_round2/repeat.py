from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-round2-inverse-probe');lock=open('/tmp/cp-fps-tuning.lock','w');result={'cpu':2,'kernel_trials':21,'whole_trials':99,'warmups':1,'shuffle_seed':1442028,'metadata':json.loads((W/'metadata.json').read_text()),'scaled_metadata':json.loads((W/'scaled_metadata.json').read_text()),'kernel':{},'whole':{},'official_checks':[],'execution_order':[]}
fcntl.flock(lock,fcntl.LOCK_SH);J=Path('/tmp/library-checker-problems/polynomial/inv_of_formal_power_series')
for inp in sorted((J/'in').glob('*.in')):
 output=W/'scaled_check_output.txt'
 with inp.open('rb')as fin,output.open('wb')as fout:subprocess.run([str(W/'scaled_inverse_whole')],stdin=fin,stdout=fout,check=True)
 subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/f'{inp.stem}.out')],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
 result['official_checks'].append({'variant':'scaled','input':inp.name,'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'pass':True})
print('official checks scaled pass',flush=True);fcntl.flock(lock,fcntl.LOCK_UN)
def save():(W/'repeat.json').write_text(json.dumps(result,indent=2)+'\n')
print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('acquired EX',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(1442028)
for n in [262144,262145,393217,500000]:
 samples={v:[]for v in ['baseline','candidate','scaled','reference']};hashes=set()
 for trial in range(22):
  order=list(samples);rng.shuffle(order);result['execution_order'].append(['kernel',n,trial,order])
  for v in order:
   binary=W/('reference_inverse_kernel'if v=='reference'else v+'_kernel');ms,h=subprocess.check_output([str(binary),str(n),'inverse'],text=True).split();hashes.add(h)
   if trial:samples[v].append(float(ms))
 assert len(hashes)==1,(n,hashes)
 row={'output_hash':next(iter(hashes)),'variants':{v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()}};result['kernel'][str(n)]=row;save();print('kernel',n,{v:r['median_ms']for v,r in row['variants'].items()},flush=True)
for op,problem,names,variants in [('inverse','inv_of_formal_power_series',['max_random_00','max_random_01'],['baseline','candidate','scaled']),('sqrt','sqrt_of_formal_power_series',['max_random_01','max_random_02'],['baseline','candidate'])]:
 J=Path('/tmp/library-checker-problems/polynomial')/problem;result['whole'][op]={}
 for name in names:
  inp=J/'in'/f'{name}.in';answer=J/'out'/f'{name}.out';assert answer.read_text().split()[0]!='-1';samples={v:[]for v in variants}
  for trial in range(100):
   order=list(samples);rng.shuffle(order);result['execution_order'].append(['whole',op,name,trial,order])
   for v in order:
    binary=W/f'{v}_{op}_whole';output=W/f'{v}_repeat_output.txt'
    with inp.open('rb')as fin,output.open('wb')as fout:
     start=time.perf_counter_ns();subprocess.run([str(binary)],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
    subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
    if trial:samples[v].append(elapsed)
  row={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'answer_sha256':hashlib.sha256(answer.read_bytes()).hexdigest(),'checker_sha256':hashlib.sha256((J/'checker').read_bytes()).hexdigest(),'variants':{v:{'median_ms':statistics.median(s),'samples_ms':s,'binary_sha256':hashlib.sha256((W/f'{v}_{op}_whole').read_bytes()).hexdigest()}for v,s in samples.items()}};result['whole'][op][name]=row;save();print('whole',op,name,{v:r['median_ms']for v,r in row['variants'].items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
