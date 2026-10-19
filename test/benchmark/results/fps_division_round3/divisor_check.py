from pathlib import Path
from datetime import datetime,timezone
import fcntl,hashlib,json,subprocess
w=Path('/tmp/fps-division-round3');source=w/'candidate/test/unit/fps_division_blocked.cpp';header=w/'candidate/components/fps_division_blocked.hpp';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();cmd=['g++','-std=c++23','-O2','-Wall','-Wextra','-Werror','-I'+str(w/'candidate'),str(source),'-o',str(w/'divisor_guard_unit')];j={'started_at':datetime.now(timezone.utc).isoformat(),'command':cmd,'source_sha256':sha(source),'header_sha256':sha(header),'harness_sha256':sha(Path(__file__))}
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH);r=subprocess.run(cmd,capture_output=True,text=True);j['compile']={'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr};assert r.returncode==0,r.stderr
 r=subprocess.run([str(w/'divisor_guard_unit')],capture_output=True,text=True);j['run']={'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr};j['binary_sha256']=sha(w/'divisor_guard_unit');j['source_unchanged']=j['source_sha256']==sha(source) and j['header_sha256']==sha(header);j['finished_at']=datetime.now(timezone.utc).isoformat();(w/'divisor_check.json').write_text(json.dumps(j,indent=2)+'\n');print(j['run'],j['source_unchanged'],flush=True);assert r.returncode==0 and j['source_unchanged']
