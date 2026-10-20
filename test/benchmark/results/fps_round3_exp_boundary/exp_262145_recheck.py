from pathlib import Path
from datetime import datetime,timezone
import argparse,fcntl,hashlib,json,os,random,statistics,subprocess,time

p=argparse.ArgumentParser()
p.add_argument('--parent',type=Path,default=Path(__file__).resolve().parents[4]/'test/benchmark/results/fps_round3_all_speed.json')
p.add_argument('--normalization',type=Path,default=Path(__file__).resolve().parents[4]/'test/benchmark/results/fps_round3_validation/bostan_pragma_normalization.json')
p.add_argument('--work',type=Path,default=Path('/workspace/cp-fps-scratch-round3/fps-round3-all-speed'))
p.add_argument('--official-root',type=Path,default=Path('/tmp/library-checker-problems'))
p.add_argument('--output',type=Path,default=Path(__file__).with_suffix('.json'))
p.add_argument('--trials',type=int,default=21)
p.add_argument('--seed',type=int,default=20261107)
a=p.parse_args();a.parent=a.parent.resolve();a.work=a.work.resolve();a.output=a.output.resolve();sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();problem='exp_of_formal_power_series';versions=['before','current','reference']
print('Waiting for completed formal timing and Bostan normalization equivalence artifact',flush=True)
while not a.normalization.exists():time.sleep(2)
normalization=json.loads(a.normalization.read_text());assert normalization['byte_identical'] and normalization['parent_raw_unchanged'];parent_sha=normalization['parent_raw']['sha256'];assert sha(a.parent)==parent_sha;parent=json.loads(a.parent.read_text());assert len(parent['judge'])==16
records={v:parent['sources'][problem+'_'+v] for v in versions};binaries={v:a.work/(problem+'_'+v) for v in versions};folder=a.official_root/'polynomial'/problem;case='near_262144_02';inp=folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out');assert inp.open().readline().strip()=='262145'

def verify():
 assert sha(a.parent)==parent_sha
 for v in versions:
  record=records[v];assert Path(record['command'][-1])==binaries[v];assert sha(binaries[v])==record['binary_sha256'];assert sha(Path(record['command'][-3]))==record['source_sha256']
 snapshots={}
 for v in ['before','current']:
  expected=parent['sources'][v] if v=='before' else normalization['normalized_sources_current'];actual={name:sha(a.work/v/name) for name in expected};assert actual==expected,v;snapshots[v]=actual
 lr=a.work/'kth_term_of_linearly_recurrent_sequence_current';assert sha(lr)==normalization['binary_after']['sha256']==parent['sources']['kth_term_of_linearly_recurrent_sequence_current']['binary_sha256']
 return {'binary_sha256':{v:sha(binaries[v]) for v in versions},'source_snapshots':snapshots}

j={'started_at':datetime.now(timezone.utc).isoformat(),'purpose':'Additional integrated exp boundary, exactly the existing three final benchmark binaries; no recompilation or production edits','parent_raw':{'path':str(a.parent),'sha256':parent_sha,'baseline':parent['baseline'],'harness_sha256':parent['harness_sha256'],'exp_default_row':next(row for row in parent['judge'] if row['problem']==problem)},'normalization_artifact':{'path':str(a.normalization),'sha256':sha(a.normalization)},'compiler':parent['compiler'],'flags':parent['flags'],'hardware':parent['hardware'],'cpu_affinity':[2],'lock':'exclusive /tmp/cp-fps-tuning.lock','metric':'Whole-process wall ms including launch, regular-file input/output, computation and exit; checker runs after timer','trials':a.trials,'warmups':1,'seed':a.seed,'harness_sha256':sha(Path(__file__)),'parent_binary_records':records,'case':case,'input':str(inp),'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'answer_sha256':sha(answer),'checker_sha256':sha(folder/'checker'),'checker_order':['input','actual','answer'],'orders_including_warmup':[],'samples_ms':{v:[] for v in versions},'checks_including_warmup':[]}
print('Waiting exclusive exp boundary timing lock',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});j['lock_acquired_at']=datetime.now(timezone.utc).isoformat();j['verified_before']=verify();rng=random.Random(a.seed)
 for rep in range(a.trials+1):
  order=versions.copy();rng.shuffle(order);j['orders_including_warmup'].append(order)
  for v in order:
   out=a.work/('exp_boundary_262145_'+v+'.out')
   with inp.open('rb') as fi,out.open('wb') as fo:
    start=time.perf_counter_ns();r=subprocess.run([str(binaries[v])],stdin=fi,stdout=fo,stderr=subprocess.PIPE,timeout=120);elapsed=(time.perf_counter_ns()-start)/1e6
   assert r.returncode==0,(v,r.stderr)
   command=[str(folder/'checker'),str(inp),str(out),str(answer)];check=subprocess.run(command,capture_output=True,text=True,timeout=120);assert check.returncode==0,(v,check.stderr)
   j['checks_including_warmup'].append({'repetition':rep,'version':v,'command':command,'returncode':check.returncode,'stdout':check.stdout,'stderr':check.stderr,'actual_sha256':sha(out)})
   if rep:j['samples_ms'][v].append(elapsed)
 j['median_ms']={v:statistics.median(x) for v,x in j['samples_ms'].items()};paired=[b-c for b,c in zip(j['samples_ms']['before'],j['samples_ms']['current'])];j['paired_savings_ms']=paired;j['paired_median_saving_ms']=statistics.median(paired);j['current_wins']=sum(d>0 for d in paired);j['verified_after']=verify();j['dependencies_unchanged']=j['verified_before']==j['verified_after'];assert j['dependencies_unchanged'];assert sha(inp)==j['input_sha256'] and sha(answer)==j['answer_sha256'] and sha(folder/'checker')==j['checker_sha256'];j['finished_at']=datetime.now(timezone.utc).isoformat();a.output.write_text(json.dumps(j,indent=2)+'\n');print(case,j['median_ms'],'paired',j['paired_median_saving_ms'],'wins',j['current_wins'],'all',len(j['checks_including_warmup']),'checker calls passed',flush=True)
