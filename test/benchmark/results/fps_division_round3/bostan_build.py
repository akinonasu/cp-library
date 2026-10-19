from pathlib import Path
import subprocess,concurrent.futures,fcntl,json,hashlib,time
w=Path('/tmp/fps-division-round3');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];j={'flags':flags,'compiler':subprocess.check_output(['g++','--version'],text=True),'commands':{},'headers':{v:{str(p.relative_to(w/v)):sha(p) for p in sorted((w/v).rglob('*.hpp'))} for v in ['base','bostan_candidate']}}
commands={v:['g++',*flags,'-I'+str(w/v),'-I'+str(w/v/'test'),str(w/v/'test/fps_linear_recurrence.test.cpp'),'-o',str(w/('bostan_'+v))] for v in ['base','bostan_candidate']}
commands['reference']=['g++',*flags,'/tmp/fps-reference/kth_term_of_linearly_recurrent_sequence/405848.cpp','-o',str(w/'bostan_reference')]
commands['unit']=['g++','-std=c++23','-O2','-Wall','-Wextra','-Werror','-I'+str(w/'bostan_candidate'),str(w/'bostan_unit.cpp'),'-o',str(w/'bostan_unit')]
def run(item):
 v,cmd=item;t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);return v,{'command':cmd,'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr,'seconds':time.monotonic()-t,'binary_sha256':sha(Path(cmd[-1])) if p.returncode==0 else None,'source_sha256':sha(Path(cmd[-3]))}
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 with concurrent.futures.ThreadPoolExecutor(2) as pool:
  for v,r in pool.map(run,commands.items()):j['commands'][v]=r;print(v,r['returncode'],r['seconds'],r['stderr'][:2000],flush=True)
 if all(r['returncode']==0 for r in j['commands'].values()):
  p=subprocess.run([str(w/'bostan_unit')],capture_output=True,text=True);j['unit_run']={'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr};print('unit run',p.returncode,p.stderr,flush=True)
(w/'bostan_build.json').write_text(json.dumps(j,indent=2)+'\n');assert all(r['returncode']==0 for r in j['commands'].values()) and j['unit_run']['returncode']==0
