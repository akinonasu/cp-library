from pathlib import Path
import shutil,subprocess,time,json,hashlib
W=Path('/tmp/fps-division-round2');C=W/'combined_candidate';B=W/'inverse_candidate'
if C.exists():shutil.rmtree(C)
shutil.copytree(B,C);shutil.copyfile(W/'candidate/components/fps_division_blocked.hpp',C/'components/fps_division_blocked.hpp')
cmd=['flock','-s','/tmp/cp-fps-tuning.lock','g++','-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell','-I',str(C),'-DMODE=1',str(W/'driver.cpp'),'-o',str(W/'inverse_mod_combined')]
t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);print(p.stdout+p.stderr,flush=True);assert p.returncode==0
result={'command':cmd,'compile_seconds':time.monotonic()-t,'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),'headers':{str(p.relative_to(C)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(C.rglob('*.hpp'))},'driver_sha256':hashlib.sha256((W/'driver.cpp').read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256((W/'inverse_mod_combined').read_bytes()).hexdigest()};(W/'combined_build.json').write_text(json.dumps(result,indent=2)+'\n');print('combined inv_mod built',flush=True)
