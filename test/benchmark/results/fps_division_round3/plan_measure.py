from pathlib import Path
from datetime import datetime,timezone
import fcntl,hashlib,json,os,platform,subprocess,statistics
w=Path('/tmp/fps-division-round3');o=Path('/tmp/library-checker-problems/polynomial/division_of_polynomials');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();j={'baseline':'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0','started_at':datetime.now(timezone.utc).isoformat(),'compiler':subprocess.check_output(['g++','--version'],text=True),'flags':['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'],'cpu_affinity':[2],'binary_sha256':sha(w/'plan_driver'),'driver_sha256':sha(w/'plan_driver.cpp'),'harness_sha256':sha(Path(__file__)),'headers':{str(p.relative_to(w/'base')):sha(p) for p in sorted((w/'base').rglob('*.hpp'))},'metric':'API kernel wall milliseconds includes per-plan arena allocation, transforms and deallocation; input and output buffers allocated before timer; all q/r words checked against default plan after timer','trials':9,'warmups':1,'plan_shuffle_seed':20261103,'lock':'exclusive /tmp/cp-fps-tuning.lock','workloads':[]}
print('Waiting plan EX',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});j['lock_acquired_at']=datetime.now(timezone.utc).isoformat()
 for case in ['v_random_00','v_random_01','v_random_02','v_random_03','n_max_00']:
  inp=o/'in'/(case+'.in')
  with inp.open('rb') as fi:r=subprocess.run([str(w/'plan_driver')],stdin=fi,capture_output=True,text=True)
  assert r.returncode==0,(case,r.stderr);row=json.loads(r.stdout);row['case']=case;row['input_sha256']=sha(inp)
  for p in row['plans']:p['median_ms']=statistics.median(p['samples_ms'])
  row['plans'].sort(key=lambda p:p['median_ms']);j['workloads'].append(row);(w/'plan_raw.json').write_text(json.dumps(j,indent=2)+'\n');print(case,'default',row['default'],'best',[(p['s'],p['remainder'],p['median_ms']) for p in row['plans'][:4]],flush=True)
 j['finished_at']=datetime.now(timezone.utc).isoformat();j['dependencies_unchanged']=j['binary_sha256']==sha(w/'plan_driver') and j['headers']=={str(p.relative_to(w/'base')):sha(p) for p in sorted((w/'base').rglob('*.hpp'))};assert j['dependencies_unchanged'];(w/'plan_raw.json').write_text(json.dumps(j,indent=2)+'\n')
