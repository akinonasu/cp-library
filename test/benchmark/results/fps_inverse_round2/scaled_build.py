from pathlib import Path
import concurrent.futures,fcntl,hashlib,json,subprocess
W=Path('/tmp/fps-round2-inverse-probe');C=Path('/tmp/fps-round2-inverse-scaled/candidate');lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH);flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];r={'flags':flags,'sources':{'scaled_headers':{str(p.relative_to(C)):hashlib.sha256(p.read_bytes()).hexdigest()for p in C.rglob('*.hpp')}}}
def build(job):
 kind,source=job;name='scaled_'+kind;binary=W/name;cmd=['g++',*flags,'-I',str(C),str(source),'-o',str(binary)];subprocess.run(cmd,check=True);return name,{'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'command':cmd}
with concurrent.futures.ThreadPoolExecutor(max_workers=2)as pool:
 for name,row in pool.map(build,[('kernel',W/'kernel.cpp'),('inverse_whole',C/'test/fps_inv.test.cpp')]):r['sources'][name]=row;print('built',name,flush=True)
(W/'scaled_metadata.json').write_text(json.dumps(r,indent=2)+'\n')
