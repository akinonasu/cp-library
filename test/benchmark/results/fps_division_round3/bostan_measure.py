from pathlib import Path
from datetime import datetime,timezone
import argparse,fcntl,hashlib,json,os,platform,random,statistics,subprocess,time
p=argparse.ArgumentParser();p.add_argument('--seed',type=int,default=20261101);p.add_argument('--trials',type=int,default=21);p.add_argument('--output',default='bostan_raw.json');a=p.parse_args()
w=Path('/tmp/fps-division-round3');o=Path('/tmp/library-checker-problems/other/kth_term_of_linearly_recurrent_sequence');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();build=json.loads((w/'bostan_build.json').read_text());variants=['base','bostan_candidate','reference'];bins={v:w/('bostan_'+v) for v in variants}
j={'started_at':datetime.now(timezone.utc).isoformat(),'baseline':'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0','build':build,'build_json_sha256':sha(w/'bostan_build.json'),'harness_sha256':sha(Path(__file__)),'platform':platform.platform(),'hardware':subprocess.check_output(['lscpu'],text=True),'trials':a.trials,'seed':a.seed,'cpu_affinity':[2],'metric':'Whole-process wall ms includes process start, I/O, allocations, API computation, output and exit; kernel stderr excludes I/O and includes allocations and result construction','lock':'exclusive /tmp/cp-fps-tuning.lock','checker_order':['input','actual','answer'],'workloads':[]}
def verify():
 for v in variants:assert sha(bins[v])==build['commands'][v]['binary_sha256'],v
 for v in ['base','bostan_candidate']:
  assert {str(p.relative_to(w/v)):sha(p) for p in sorted((w/v).rglob('*.hpp'))}==build['headers'][v],v
 assert sha(w/'bostan_build.json')==j['build_json_sha256']
def save():(w/a.output).write_text(json.dumps(j,indent=2)+'\n')
print('Waiting EX',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});verify();rng=random.Random(a.seed);j['lock_acquired_at']=datetime.now(timezone.utc).isoformat()
 for case in ['max_random_00','max_random_01','max_random_02','near_65536_00','near_65536_01','near_65536_02']:
  inp=o/'in'/(case+'.in');ans=o/'out'/(case+'.out');whole={v:[] for v in variants};kernel={v:[] for v in variants if v!='reference'};checks=[];orders=[]
  for rep in range(a.trials+1):
   order=variants.copy();rng.shuffle(order);orders.append(order)
   for v in order:
    out=w/('bostan_actual_'+v+'.out')
    with inp.open('rb') as fi,out.open('wb') as fo:
     start=time.perf_counter_ns();r=subprocess.run([str(bins[v])],stdin=fi,stdout=fo,stderr=subprocess.PIPE,timeout=120);elapsed=(time.perf_counter_ns()-start)/1e6
    assert r.returncode==0,(case,v,r.stderr)
    check=subprocess.run([str(o/'checker'),str(inp),str(out),str(ans)],capture_output=True,text=True);assert check.returncode==0,(case,v,check.stderr)
    checks.append({'repetition':rep,'variant':v,'returncode':check.returncode,'stdout':check.stdout,'stderr':check.stderr,'actual_sha256':sha(out)})
    if rep:
     whole[v].append(elapsed)
     if v!='reference' and r.stderr:kernel[v].append(float(r.stderr))
  def summary(s):
   pair=[b-c for b,c in zip(s['base'],s['bostan_candidate'])]
   return {'samples_ms':s,'medians_ms':{v:statistics.median(x) for v,x in s.items()},'paired_savings_ms':pair,'paired_median_saving_ms':statistics.median(pair),'candidate_wins':sum(d>0 for d in pair)}
  row={'case':case,'input':str(inp),'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'answer_sha256':sha(ans),'checker_sha256':sha(o/'checker'),'orders_including_warmup':orders,'whole':summary(whole),'kernel':None,'checks_including_warmup':checks};j['workloads'].append(row);save();print(case,row['whole']['medians_ms'],'paired',row['whole']['paired_median_saving_ms'],'wins',row['whole']['candidate_wins'],flush=True)
 verify();j['dependencies_unchanged']=True;j['finished_at']=datetime.now(timezone.utc).isoformat();save()
