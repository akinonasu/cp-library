from pathlib import Path
import subprocess,concurrent.futures,fcntl,json,hashlib,time
w=Path('/tmp/fps-division-round3');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
commands={v:['g++',*flags,'-DMODE=0','-I'+str(w/v),str(w/'driver.cpp'),'-o',str(w/('division_'+v))] for v in ['base','candidate']}
commands['reference']=['g++',*flags,'/tmp/fps-reference/division_of_polynomials/409561.cpp','-o',str(w/'division_reference')]
j={'flags':flags,'baseline':subprocess.check_output(['git','rev-parse','bd951dc'],text=True).strip(),'compiler':subprocess.check_output(['g++','--version'],text=True),'commands':{},'headers':{v:{str(p.relative_to(w/v)):sha(p) for p in sorted((w/v).rglob('*.hpp'))} for v in ['base','candidate']}}
def run(item):
 v,cmd=item;t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);return v,{'command':cmd,'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr,'seconds':time.monotonic()-t,'binary_sha256':sha(Path(cmd[-1])) if p.returncode==0 else None,'source_sha256':sha(Path(cmd[-3]))}
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 with concurrent.futures.ThreadPoolExecutor(2) as pool:
  for v,r in pool.map(run,commands.items()):j['commands'][v]=r;print(v,r['returncode'],r['seconds'],r['stderr'][:2000],flush=True)
(w/'build.json').write_text(json.dumps(j,indent=2)+'\n')
assert all(r['returncode']==0 for r in j['commands'].values())
