from pathlib import Path
import concurrent.futures,fcntl,hashlib,json,subprocess
W=Path('/tmp/fps-round2-inverse-probe');lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH);flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];r={'flags':flags,'sources':{},'tests':{}};jobs=[]
for name,root in [('fused',Path('/tmp/fps-round2-inverse-fused-candidate')),('combined',Path('/tmp/fps-round2-inverse-combined-candidate'))]:
 r['sources'][name+'_headers']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest()for p in root.rglob('*.hpp')}
 for kind,source in [('kernel',W/'kernel.cpp'),('inverse_whole',root/'test/fps_inv.test.cpp'),('unit',root/'test/unit/fps_inverse_blocked.cpp')]:jobs.append((name+'_'+kind,source,root,flags))
 if name=='fused':
  for kind,extra in [('check',[]),('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:jobs.append(('fused_'+kind,W/'fused_check.cpp',root,flags+extra))
def build(job):
 name,source,root,options=job;binary=W/name;cmd=['g++',*options,'-I',str(root),str(source),'-o',str(binary)];subprocess.run(cmd,check=True);return name,{'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'command':cmd}
with concurrent.futures.ThreadPoolExecutor(max_workers=3)as pool:
 for name,row in pool.map(build,jobs):r['sources'][name]=row;print('built',name,flush=True)
for name in ['fused_unit','combined_unit','fused_check','fused_sanitize']:
 subprocess.run([str(W/name)],check=True);r['tests'][name]='passed';print('passed',name,flush=True)
(W/'fused_metadata.json').write_text(json.dumps(r,indent=2)+'\n')
