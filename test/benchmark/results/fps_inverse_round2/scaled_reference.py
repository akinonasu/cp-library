from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
W=Path('/tmp/fps-round2-inverse-probe');J=Path('/tmp/library-checker-problems/polynomial/inv_of_formal_power_series');lock=open('/tmp/cp-fps-tuning.lock','w');r={'cpu':2,'trials':21,'warmups':1,'shuffle_seed':1442029,'whole':{},'execution_order':[]};print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);print('acquired EX',flush=True);os.sched_setaffinity(0,{2});rng=random.Random(1442029)
for name in ['max_random_00','max_random_01']:
 inp=J/'in'/f'{name}.in';answer=J/'out'/f'{name}.out';samples={v:[]for v in ['baseline','scaled','reference']}
 for trial in range(22):
  order=list(samples);rng.shuffle(order);r['execution_order'].append([name,trial,order])
  for v in order:
   binary=W/('reference_inverse_whole'if v=='reference'else v+'_inverse_whole');output=W/f'{v}_reference_output.txt'
   with inp.open('rb')as fin,output.open('wb')as fout:
    start=time.perf_counter_ns();subprocess.run([str(binary)],stdin=fin,stdout=fout,check=True);elapsed=(time.perf_counter_ns()-start)/1e6
   subprocess.run([str(J/'checker'),str(inp),str(output),str(answer)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
   if trial:samples[v].append(elapsed)
 r['whole'][name]={'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'answer_sha256':hashlib.sha256(answer.read_bytes()).hexdigest(),'checker_sha256':hashlib.sha256((J/'checker').read_bytes()).hexdigest(),'variants':{v:{'median_ms':statistics.median(s),'samples_ms':s,'binary_sha256':hashlib.sha256((W/('reference_inverse_whole'if v=='reference'else v+'_inverse_whole')).read_bytes()).hexdigest()}for v,s in samples.items()}};(W/'scaled_reference.json').write_text(json.dumps(r,indent=2)+'\n');print(name,{v:row['median_ms']for v,row in r['whole'][name]['variants'].items()},flush=True)
fcntl.flock(lock,fcntl.LOCK_UN)
