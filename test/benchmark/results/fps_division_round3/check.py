from pathlib import Path
from datetime import datetime,timezone
import concurrent.futures,fcntl,hashlib,json,os,subprocess,time
w=Path('/tmp/fps-division-round3');tree=w/'bostan_candidate';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();g13='/tmp/fps-gcc13/toolchain/usr/bin/g++-13';clang='/tmp/fps-clang/toolchain/usr/bin/clang++-19';(w/'bostan_standalone.cpp').write_text('#include "components/fps_bostan.hpp"\nint main() {}\n')
normal=['-std=c++23','-O2','-Wall','-Wextra','-Werror','-I'+str(tree)];san=['-std=c++23','-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(tree)]
commands={'standalone':['g++',*normal,str(w/'bostan_standalone.cpp'),'-o',str(w/'check_standalone')],'unit':['g++',*normal,str(tree/'test/unit/fps_bostan.cpp'),'-o',str(w/'check_unit')],'algorithms':['g++',*normal,str(tree/'test/unit/fps_algorithms.cpp'),'-o',str(w/'check_algorithms')],'unit_san':['g++',*san,str(tree/'test/unit/fps_bostan.cpp'),'-o',str(w/'check_unit_san')],'algorithms_san':['g++',*san,str(tree/'test/unit/fps_algorithms.cpp'),'-o',str(w/'check_algorithms_san')],'gcc13_standalone':[g13,*normal,str(w/'bostan_standalone.cpp'),'-o',str(w/'check_gcc13_standalone')],'gcc13_unit':[g13,*normal,str(tree/'test/unit/fps_bostan.cpp'),'-o',str(w/'check_gcc13_unit')],'clang_standalone':[clang,*normal,str(w/'bostan_standalone.cpp'),'-o',str(w/'check_clang_standalone')],'clang_algorithms':[clang,*normal,str(tree/'test/unit/fps_algorithms.cpp'),'-o',str(w/'check_clang_algorithms')]}
j={'started_at':datetime.now(timezone.utc).isoformat(),'purpose':'Bostan fused out-of-place inverse and final twist, fixed private bd951 snapshot plus candidate patch','headers_before':{str(p.relative_to(tree)):sha(p) for p in sorted(tree.rglob('*.hpp'))},'compilers':{c:subprocess.check_output([c,'--version'],text=True).splitlines()[0] for c in ['g++',g13,clang]},'builds':{},'runs':{},'official':[],'harness_sha256':sha(Path(__file__))}
def save():(w/'check.json').write_text(json.dumps(j,indent=2)+'\n')
def build(item):
 name,cmd=item;start=time.monotonic();r=subprocess.run(cmd,capture_output=True,text=True);return name,{'command':cmd,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'seconds':time.monotonic()-start,'source_sha256':sha(Path(cmd[-3])),'binary_sha256':sha(Path(cmd[-1])) if r.returncode==0 else None}
print('Waiting check SH',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH);j['lock_acquired_at']=datetime.now(timezone.utc).isoformat()
 with concurrent.futures.ThreadPoolExecutor(2) as pool:
  for name,r in pool.map(build,commands.items()):j['builds'][name]=r;save();print(name,r['returncode'],r['seconds'],r['stderr'][:800],flush=True)
 assert all(r['returncode']==0 for r in j['builds'].values())
 for name,r in j['builds'].items():
  cmd=[r['command'][-1]];env=os.environ|{'ASAN_OPTIONS':'detect_leaks=0','UBSAN_OPTIONS':'halt_on_error=1'};r=subprocess.run(cmd,capture_output=True,text=True,env=env,timeout=120);j['runs'][name]={'command':cmd,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr};save();print('run',name,r.returncode,r.stderr[:800],flush=True);assert r.returncode==0
 folder=Path('/tmp/library-checker-problems/other/kth_term_of_linearly_recurrent_sequence')
 for inp in sorted((folder/'in').glob('*.in')):
  out=w/'official_bostan.out';answer=folder/'out'/(inp.stem+'.out')
  with inp.open('rb') as fi,out.open('wb') as fo:r=subprocess.run([str(w/'bostan_bostan_candidate')],stdin=fi,stdout=fo,stderr=subprocess.PIPE)
  assert r.returncode==0
  cmd=[str(folder/'checker'),str(inp),str(out),str(answer)];r=subprocess.run(cmd,capture_output=True,text=True);j['official'].append({'case':inp.name,'input_sha256':sha(inp),'answer_sha256':sha(answer),'actual_sha256':sha(out),'command':cmd,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr});save();assert r.returncode==0,(inp,r.stderr)
 j['headers_after']={str(p.relative_to(tree)):sha(p) for p in sorted(tree.rglob('*.hpp'))};j['dependencies_unchanged']=j['headers_before']==j['headers_after'];assert j['dependencies_unchanged'];j['finished_at']=datetime.now(timezone.utc).isoformat();save();print('All checks passed',len(j['official']),'official cases',flush=True)
