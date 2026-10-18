from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-round2-inverse-probe');B=Path('/tmp/cp-fps-round2-base');C=Path('/tmp/fps-round2-inverse-candidate');lock=open('/tmp/cp-fps-tuning.lock','w');result=json.loads((W/'metadata.json').read_text());result.update({'trials':21,'warmups':1,'shuffle_seed':1442026,'append':{},'kernel':{},'whole':{},'official_checks':[],'execution_order':[]})
fcntl.flock(lock,fcntl.LOCK_SH)
for v,root in [('baseline',B),('candidate',C)]:result['sources'][v+'_headers']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest()for p in root.rglob('*.hpp')}
for op,problem in [('inverse','inv_of_formal_power_series'),('sqrt','sqrt_of_formal_power_series')]:
 J=Path('/tmp/library-checker-problems/polynomial')/problem
 for inp in sorted((J/'in').glob('*.in')):
  output=W/'official_output.txt'
  with inp.open('rb')as fin,output.open('wb')as fout:subprocess.run([str(W/('candidate_'+op+'_whole'))],stdin=fin,stdout=fout,check=True)
  subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/f'{inp.stem}.out')],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
  result['official_checks'].append({'problem':problem,'input':inp.name,'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'pass':True})
 print('official checks',op,'pass',flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
def save(): (W/'results.json').write_text(json.dumps(result,indent=2)+'\n')
def kernel_trials(case,commands):
 samples={name:[]for name in commands};hashes=set()
 for trial in range(22):
  order=list(commands);rng.shuffle(order);result['execution_order'].append([case,trial,order])
  for name in order:
   ms,h=subprocess.check_output(commands[name],text=True).split();hashes.add(h)
   if trial:samples[name].append(float(ms))
 assert len(hashes)==1,(case,hashes)
 return {'output_hash':next(iter(hashes)),'variants':{v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()}}
print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('acquired EX',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(1442026)
for scale in [65536,932051910]:
 commands={v:[str(W/(v+'_append_kernel')),'500000',str(scale)]for v in ['baseline','candidate']};row=kernel_trials('append '+str(scale),commands);result['append'][str(scale)]=row;save();print('append',scale,{v:r['median_ms']for v,r in row['variants'].items()},flush=True)
for op in ['inverse','sqrt']:
 result['kernel'][op]={}
 for n in [262144,262145,393217,500000]:
  commands={v:[str(W/(v+'_kernel')),str(n),op]for v in ['baseline','candidate']};commands['reference']=[str(W/('reference_'+op+'_kernel')),str(n)];row=kernel_trials(op+' '+str(n),commands);result['kernel'][op][str(n)]=row;save();print('kernel',op,n,{v:r['median_ms']for v,r in row['variants'].items()},flush=True)
for op,problem in [('inverse','inv_of_formal_power_series'),('sqrt','sqrt_of_formal_power_series')]:
 result['whole'][op]={};J=Path('/tmp/library-checker-problems/polynomial')/problem
 for name in ['max_random_00','near_262144_00']:
  inp=J/'in'/f'{name}.in';answer=J/'out'/f'{name}.out';samples={v:[]for v in ['baseline','candidate','reference']}
  for trial in range(22):
   order=list(samples);rng.shuffle(order);result['execution_order'].append(['whole '+op+' '+name,trial,order])
   for v in order:
    binary=W/(f'reference_{op}_whole'if v=='reference'else f'{v}_{op}_whole');output=W/f'{v}_output.txt'
    with inp.open('rb')as fin,output.open('wb')as fout:
     start=time.perf_counter_ns();subprocess.run([str(binary)],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
    subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
    if trial:samples[v].append(elapsed)
  row={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'answer_sha256':hashlib.sha256(answer.read_bytes()).hexdigest(),'checker_sha256':hashlib.sha256((J/'checker').read_bytes()).hexdigest(),'variants':{v:{'median_ms':statistics.median(s),'samples_ms':s}for v,s in samples.items()}};result['whole'][op][name]=row;save();print('whole',op,name,{v:r['median_ms']for v,r in row['variants'].items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
