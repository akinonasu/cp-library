from pathlib import Path
import subprocess,time,json,hashlib
W=Path('/tmp/fps-division-round2');B=Path('/tmp/cp-fps-round2-base');C=W/'dividend_scoped_candidate';flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell','-DMODE=3'];j={}
for name,tree in [('series_round2_baseline',B),('series_final',C)]:
 cmd=['flock','-s','/tmp/cp-fps-tuning.lock','g++',*flags,'-I',str(tree),str(W/'driver.cpp'),'-o',str(W/name)];t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);assert p.returncode==0,p.stderr;j[name]={'command':cmd,'seconds':time.monotonic()-t,'output':p.stdout+p.stderr,'binary_sha256':hashlib.sha256((W/name).read_bytes()).hexdigest()};print(name,j[name]['seconds'],flush=True)
(W/'series_build.json').write_text(json.dumps(j,indent=2)+'\n')
