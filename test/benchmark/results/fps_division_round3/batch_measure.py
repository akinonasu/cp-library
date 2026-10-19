from pathlib import Path
from datetime import datetime,timezone
import fcntl,hashlib,json,os,subprocess,statistics
w=Path('/tmp/fps-division-round3');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
print('Waiting batch EX',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2})
 j={'started_at':datetime.now(timezone.utc).isoformat(),'driver_sha256':sha(w/'batch_driver.cpp'),'binary_sha256':sha(w/'batch_driver'),'harness_sha256':sha(Path(__file__)),'compiler':subprocess.check_output(['g++','--version'],text=True),'flags':['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'],'cpu_affinity':[2],'trials':41,'warmups':3,'seed':20261104,'metric':'micro API result construction wall ms; includes two result vector allocations, reserve and append; excludes destruction and exact .v equality checks after each sample','baseline_header_sha256':sha(w/'base/components/fps_division_blocked.hpp')}
 r=subprocess.run([str(w/'batch_driver')],capture_output=True,text=True);assert r.returncode==0,r.stderr;j['workloads']=json.loads(r.stdout)
 for row in j['workloads']:
  for variant in row['variants']:variant['median_ms']=statistics.median(variant['samples_ms'])
  print(row['k'],row['d'],[(v['batch'],v['median_ms']) for v in row['variants']],flush=True)
 j['finished_at']=datetime.now(timezone.utc).isoformat();j['dependencies_unchanged']=j['driver_sha256']==sha(w/'batch_driver.cpp') and j['binary_sha256']==sha(w/'batch_driver');assert j['dependencies_unchanged'];(w/'batch_micro_raw.json').write_text(json.dumps(j,indent=2)+'\n')
