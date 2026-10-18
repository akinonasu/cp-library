from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import subprocess,time,json,hashlib
W=Path('/tmp/fps-division-round2');C=W/'scoped_candidate';j={'headers':{str(p.relative_to(C)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(C.rglob('*.hpp'))},'builds':{}}
def build(mode):
 name=['division','inverse_mod'][mode]+'_scoped';cmd=['flock','-s','/tmp/cp-fps-tuning.lock','g++','-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell','-I',str(C),'-DMODE='+str(mode),str(W/'driver.cpp'),'-o',str(W/name)];t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr;return name,{'command':cmd,'seconds':time.monotonic()-t,'output':p.stdout+p.stderr,'binary_sha256':hashlib.sha256((W/name).read_bytes()).hexdigest()}
with ThreadPoolExecutor(max_workers=2) as pool:
 for name,result in pool.map(build,[0,1]):j['builds'][name]=result;print(name,result['seconds'],flush=True)
(W/'scoped_build.json').write_text(json.dumps(j,indent=2)+'\n')
