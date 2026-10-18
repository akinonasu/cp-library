from pathlib import Path
import concurrent.futures,fcntl,hashlib,json,subprocess
W=Path('/tmp/fps-round2-inverse-probe');B=Path('/tmp/cp-fps-round2-base');C=Path('/tmp/fps-round2-inverse-candidate');flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH)
r={'baseline':'144ae323','cpu':2,'flags':flags,'assertions':True,'sources':{}}
for op,num,marker in [('inverse',409370,'namespace {\n\n// g = 1 / f'),('sqrt',409728,'namespace {\n\nconstexpr std::uint32_t kP = poly::kModulus;')]:
 p=Path('/tmp/fps-reference')/('inv_of_formal_power_series'if op=='inverse'else'sqrt_of_formal_power_series')/f'{num}.cpp';s=p.read_text();(W/f'reference_{op}_kernel.cpp').write_text(s[:s.index(marker)]+(W/f'ref_{op}_tail.cpp').read_text());r['sources'][f'reference_{op}_whole']={'source':str(p),'source_sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
jobs=[]
for v,root in [('baseline',B),('candidate',C)]:
 for kind,source in [('kernel',W/'kernel.cpp'),('append_kernel',W/'append_kernel.cpp'),('inverse_whole',root/'test/fps_inv.test.cpp'),('sqrt_whole',root/'test/fps_sqrt.test.cpp')]:jobs.append((v+'_'+kind,source,root,flags))
 r['sources'][v+'_headers']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest()for p in root.rglob('*.hpp')}
for op,num in [('inverse',409370),('sqrt',409728)]:
 for kind in ['kernel','whole']:
  source=W/f'reference_{op}_kernel.cpp'if kind=='kernel'else Path(r['sources'][f'reference_{op}_whole']['source']);jobs.append((f'reference_{op}_{kind}',source,W,flags))
for name,source,extra in [('unit',C/'test/unit/fps_inverse_blocked.cpp',[]),('append_check',W/'append_check.cpp',[]),('unit_sanitize',C/'test/unit/fps_inverse_blocked.cpp',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer']),('append_sanitize',W/'append_check.cpp',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all','-fno-omit-frame-pointer'])]:jobs.append((name,source,C,flags+extra))
def build(job):
 name,source,root,options=job;binary=W/name;cmd=['g++',*options,'-I',str(root),str(source),'-o',str(binary)];subprocess.run(cmd,check=True);record={'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'command':cmd};return name,record
with concurrent.futures.ThreadPoolExecutor(max_workers=3)as pool:
 for name,record in pool.map(build,jobs):r['sources'][name]=record;print('built',name,flush=True)
for name in ['unit','append_check','unit_sanitize','append_sanitize']:
 subprocess.run([str(W/name)],check=True);print('passed',name,flush=True)
for v,root in [('baseline',B),('candidate',C)]:r['sources'][v+'_headers']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest()for p in root.rglob('*.hpp')}
r['compiler']=subprocess.check_output(['g++','--version'],text=True);r['cpu_model']=subprocess.check_output(['lscpu'],text=True)
(W/'metadata.json').write_text(json.dumps(r,indent=2)+'\n')
