from pathlib import Path
import hashlib,json,re,importlib.util,subprocess,fcntl,os,time
R=Path('/workspace/cp-library');O=R/'test/benchmark/results/fps_source_size';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
changed={(R/x).resolve() for x in ['components/fps_asm.hpp','components/fps_blocked.hpp','components/fps_composition.hpp','components/fps_convolution.hpp','components/fps_sqrt_blocked.hpp']};cache={}
def closure(p):
 if p in cache:return cache[p]
 seen={p};cache[p]=seen
 for a,b in re.findall(r'^\s*#\s*include\s*(?:"([^"]+)"|<([^>]+)>)',p.read_text(),re.M):
  for q in ([p.parent/a,R/a] if a else [R/b]):
   if q.is_file():seen.update(closure(q.resolve()));break
 return seen
spec=importlib.util.spec_from_file_location('size_ci',R/'.verify-helper/ci.py');ci=importlib.util.module_from_spec(spec);spec.loader.exec_module(ci);plan=ci.build_plan(R,environment='fps-source-size-docs')
headers=[name for name in plan['headers'] if closure((R/name).resolve())&changed];plan.update(headers=headers,bundles=headers,unit=[],sanitize=[],verification=json.loads((O/'drivers.json').read_text()),reason='Only FPS assembly representation dependencies',full=False)
selection=O/'selection.json';selection.write_text(json.dumps(plan,indent=2)+'\n');P='/tmp/fps-venv/bin/python';env=os.environ.copy();env['PATH']='/tmp/cp-fps-docs-bin:'+env['PATH']
commands=[[P,'.verify-helper/ci.py','docs','--selection',str(selection.relative_to(R))],[P,'.verify-helper/check-docs.py','--selection',str(selection.relative_to(R))],['python3','test/verify.py','--unit-only','--cxx','/usr/bin/g++','/tmp/fps-clang/toolchain/usr/bin/clang++-19','--selection',str(selection.relative_to(R))]]
files={p for folder in ['components','fps','convolution','math','data_structure','graph','string','docs'] for p in (R/folder).rglob('*') if p.is_file()};files.add(R/'README.md');before={str(p.relative_to(R)):sha(p) for p in files};result={'selection_sha256':sha(selection),'selected_headers':headers,'commands':[],'source_hashes_before':before}
for i,command in enumerate(commands):
 log=O/('docs'+str(i)+'.log');begin=time.monotonic()
 with open('/tmp/cp-fps-tuning.lock','a') as lock,log.open('w') as out:
  fcntl.flock(lock,fcntl.LOCK_SH);x=subprocess.run(command,cwd=R,env=env,stdout=out,stderr=subprocess.STDOUT)
 result['commands'].append({'command':command,'returncode':x.returncode,'log':log.name,'log_sha256':sha(log),'seconds':time.monotonic()-begin});assert x.returncode==0,(command,log)
result['source_hashes_after']={str(p.relative_to(R)):sha(p) for p in files};assert result['source_hashes_before']==result['source_hashes_after'];result['status']='passed';(O/'docs.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS documentation and GCC/Clang standalone headers',len(headers),flush=True)
