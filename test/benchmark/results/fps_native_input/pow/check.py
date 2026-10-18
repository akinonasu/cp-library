from pathlib import Path
import fcntl,hashlib,json,subprocess,time
W=Path('/tmp/fps-pow-native-probe');root=W/'candidate';lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH)
base=['g++','-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell','-I',str(root)]
checks=[]
for name,source,flags in [('safety',W/'safety.cpp',[]),('safety_sanitize',W/'safety.cpp',['-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer','-g']),('unit_sanitize',root/'test/unit/fps_exp_blocked.cpp',['-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer','-g'])]:
 binary=W/name;cmd=[*base,*flags,str(source),'-o',str(binary)];subprocess.run(cmd,check=True);start=time.time();subprocess.run([str(binary)],check=True);checks.append({'name':name,'command':cmd,'elapsed_s':time.time()-start,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'pass':True});print('passed',name,flush=True)
J=Path('/tmp/library-checker-problems/polynomial/pow_of_formal_power_series');output=W/'check_output.txt'
for inp in sorted((J/'in').glob('*.in')):
 with inp.open('rb')as fin,output.open('wb')as fout:subprocess.run([str(W/'candidate_whole')],stdin=fin,stdout=fout,check=True)
 subprocess.run([str(J/'checker'),str(inp),str(output),str(J/'out'/f'{inp.stem}.out')],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 print('official',inp.name,'pass',flush=True)
 checks.append({'name':inp.name,'input_sha256':hashlib.sha256(inp.read_bytes()).hexdigest(),'pass':True})
(W/'checks.json').write_text(json.dumps(checks,indent=2)+'\n')
