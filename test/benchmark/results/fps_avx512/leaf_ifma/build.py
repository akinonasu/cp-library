from pathlib import Path
import concurrent.futures,fcntl,hashlib,json,subprocess
W=Path('/tmp/fps-ifma-leaf-probe');flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH);r={'baseline':'bd951dce6fdcb3ea9db4fcef342147012a8ab0c0','flags':flags,'cpu':2,'sources':{},'compiler':subprocess.check_output(['g++','--version'],text=True),'cpu_model':subprocess.check_output(['lscpu'],text=True)}
for p in [W/'leaf_ifma.hpp',W/'base/components/fps_blocked.hpp']:r['sources'][str(p)]={'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
def build(name):
 source=W/(name+'.cpp');binary=W/name;cmd=['g++',*flags,'-I',str(W/'base'),str(source),'-o',str(binary)];subprocess.run(cmd,check=True);return name,{'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'command':cmd}
with concurrent.futures.ThreadPoolExecutor(max_workers=2)as pool:
 for name,row in pool.map(build,['check','micro']):r['sources'][name]=row;print('built',name,flush=True)
r['check_output']=subprocess.check_output([str(W/'check')],text=True);print(r['check_output'],flush=True);(W/'metadata.json').write_text(json.dumps(r,indent=2)+'\n')
