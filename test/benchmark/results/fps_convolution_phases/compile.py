from pathlib import Path
import fcntl,json,subprocess,concurrent.futures,hashlib
w=Path(__file__).resolve().parent;tree=w/'source';flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
sources={'current':tree/'test/convolution.test.cpp','reference':w/'reference_original.cpp','profile_current':w/'profile_current.cpp','profile_reference':w/'profile_reference.cpp','clock_calibration':w/'clock_calibration.cpp'}
def build(pair):
 name,source=pair;binary=w/name;command=['g++',*flags,'-I',str(tree),'-I',str(w),str(source),'-o',str(binary)]
 subprocess.run(command,check=True)
 return name,{'command':command,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest()}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:result=dict(pool.map(build,sources.items()))
(w/'build.json').write_text(json.dumps(result,indent=2)+'\n')
print('Built',list(result),flush=True)
