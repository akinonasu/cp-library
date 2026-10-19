from pathlib import Path
from datetime import datetime,timezone
import fcntl,hashlib,json,os,platform,random,statistics,subprocess,time
w=Path('/workspace/cp-fps-scratch-round3/mint-inline');o=Path('/tmp/library-checker-problems');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();snapshot=json.loads((w/'snapshot.json').read_text());seed=20261105
print('Waiting timing EX',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});build=json.loads((w/'build.json').read_text())
 def verify():
  for v in ['base','candidate']:
   assert snapshot['headers'][v]=={f:sha(w/v/f) for f in snapshot['headers'][v]},v
  for name,record in build['builds'].items():
   if name.startswith(('conv_','exp_','bostan_')):assert sha(w/name)==record['binary_sha256'],name
 verify();j={'started_at':datetime.now(timezone.utc).isoformat(),'harness_sha256':sha(Path(__file__)),'snapshot_sha256':sha(w/'snapshot.json'),'build_sha256':sha(w/'build.json'),'compiler':build['compiler'],'flags':build['flags'],'hardware':subprocess.check_output(['lscpu'],text=True),'cpu_affinity':[2],'lock':'exclusive /tmp/cp-fps-tuning.lock','trials':9,'warmups':1,'seed':seed,'metric':'Whole-process wall milliseconds including launch, input, computation, output, and exit; checker after timer','checker_order':['input','actual','answer'],'workloads':[]};rng=random.Random(seed)
 cases=[('conv','convolution/convolution_mod','max_random_00'),('exp','polynomial/exp_of_formal_power_series','max_random_00'),('bostan','other/kth_term_of_linearly_recurrent_sequence','max_random_00'),('bostan','other/kth_term_of_linearly_recurrent_sequence','near_65536_00')]
 for label,folder,case in cases:
  folder=o/folder;inp=folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out');samples={v:[] for v in ['base','candidate']};orders=[];checks=[]
  for rep in range(10):
   order=['base','candidate'];rng.shuffle(order);orders.append(order)
   for v in order:
    out=w/(label+'_'+v+'.out')
    with inp.open('rb') as fi,out.open('wb') as fo:
     start=time.perf_counter_ns();r=subprocess.run([str(w/(label+'_'+v))],stdin=fi,stdout=fo,stderr=subprocess.PIPE,timeout=120);elapsed=(time.perf_counter_ns()-start)/1e6
    assert r.returncode==0,(label,case,v,r.stderr)
    command=[str(folder/'checker'),str(inp),str(out),str(answer)];c=subprocess.run(command,capture_output=True,text=True);assert c.returncode==0,(command,c.stderr)
    checks.append({'rep':rep,'variant':v,'returncode':c.returncode,'actual_sha256':sha(out),'stdout':c.stdout,'stderr':c.stderr})
    if rep:samples[v].append(elapsed)
  pair=[b-c for b,c in zip(samples['base'],samples['candidate'])];row={'label':label,'case':case,'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'answer_sha256':sha(answer),'checker_sha256':sha(folder/'checker'),'orders_including_warmup':orders,'samples_ms':samples,'medians_ms':{v:statistics.median(x) for v,x in samples.items()},'paired_savings_ms':pair,'paired_median_saving_ms':statistics.median(pair),'candidate_wins':sum(x>0 for x in pair),'checks_including_warmup':checks};j['workloads'].append(row);(w/'whole_raw.json').write_text(json.dumps(j,indent=2)+'\n');print(label,case,row['medians_ms'],'paired',row['paired_median_saving_ms'],'wins',row['candidate_wins'],flush=True)
 verify();j['dependencies_unchanged']=True;j['finished_at']=datetime.now(timezone.utc).isoformat();(w/'whole_raw.json').write_text(json.dumps(j,indent=2)+'\n')
