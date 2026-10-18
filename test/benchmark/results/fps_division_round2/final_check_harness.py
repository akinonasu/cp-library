from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
import os,fcntl,subprocess,time,json,hashlib,shutil
W=Path('/tmp/fps-division-round2');R=Path('/workspace/cp-library');B=Path('/tmp/cp-fps-round2-base');C=W/'dividend_scoped_candidate';O=Path('/tmp/library-checker-problems/polynomial');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_SH)
shutil.copyfile(R/'test/unit/fps_division_blocked.cpp',W/'final_unit_division.cpp');s=(W/'final_unit_division.cpp').read_text().replace('{division_detail::append_words, division_detail::append_division_words}','{division_detail::append_words}');(W/'baseline_unit_division.cpp').write_text(s)
bench=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];unit=['-std=c++23','-O2','-Wall','-Wextra','-Werror','-DDEBUG=1'];j={'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','candidate_tree':str(C),'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'headers_before':{str(p.relative_to(C)):sha(p) for p in sorted(C.rglob('*.hpp'))},'builds':{},'runs':{},'official':[]}
previous=json.loads((W/'final_check.json').read_text()) if (W/'final_check.json').exists() else {}
jobs=[]
for mode,name in enumerate(['division','inverse_mod','gcd']):jobs.append((name+'_final',C,W/'driver.cpp',bench+['-DMODE='+str(mode)],True))
jobs += [('series_final',C,W/'driver.cpp',bench+['-DMODE=3'],True)]
jobs += [('standalone_final',C,W/'standalone.cpp',unit+['-fsyntax-only'],False),('division_unit_final',C,W/'final_unit_division.cpp',unit,True),('gcd_unit_final',C,W/'unit_gcd.cpp',unit,True),('division_sanitize_final',C,W/'final_unit_division.cpp',unit+['-fsanitize=address,undefined','-fno-omit-frame-pointer','-g'],True),('division_baseline_alias',B,W/'baseline_unit_division.cpp',unit,True)]
def save():(W/'final_check.json').write_text(json.dumps(j,indent=2)+'\n')
def build(job):
 name,tree,source,flags,binary=job
 cached=previous.get('builds',{}).get(name)
 if cached and cached['returncode']==0 and cached['source_sha256']==sha(source) and (tree==B or previous.get('headers_after')==j['headers_before']) and (not binary or cached.get('binary_sha256')==sha(W/name)):
  return name,{**cached,'reused_successful_build':True}
 cmd=['g++',*flags,'-I',str(tree),str(source),*(['-o',str(W/name)] if binary else [])];t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);info={'command':cmd,'source_sha256':sha(source),'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr,'seconds':time.monotonic()-t}
 if binary and p.returncode==0:info['binary_sha256']=sha(W/name)
 return name,info
with ThreadPoolExecutor(max_workers=2) as pool:
 for name,info in pool.map(build,jobs):j['builds'][name]=info;save();print('build',name,info['returncode'],info['seconds'],info['stderr'],flush=True)
assert all(info['returncode']==0 for info in j['builds'].values())
for name in ['division_unit_final','gcd_unit_final','division_sanitize_final','division_baseline_alias']:
 if name=='division_baseline_alias' and previous.get('runs',{}).get(name,{}).get('returncode')==0 and j['builds'][name].get('reused_successful_build'):
  j['runs'][name]={**previous['runs'][name],'reused_successful_run':True};print('run reused',name,flush=True);continue
 env=os.environ.copy();env.update(ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1');t=time.monotonic();p=subprocess.run([str(W/name)],capture_output=True,text=True,env=env);j['runs'][name]={'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr,'seconds':time.monotonic()-t};save();print('run',name,p.returncode,p.stdout,p.stderr,flush=True);assert p.returncode==0
for label,problem in [('division','division_of_polynomials'),('inverse_mod','inv_of_polynomials')]:
 folder=O/problem
 for inp in sorted((folder/'in').glob('*.in')):
  answer=folder/'out'/(inp.stem+'.out');out=W/'official_actual.out'
  with inp.open('rb') as fi,out.open('wb') as fo:p=subprocess.run([str(W/(label+'_final'))],stdin=fi,stdout=fo,stderr=subprocess.PIPE)
  assert p.returncode==0,p.stderr
  cmd=[str(folder/'checker'),str(inp),str(out),str(answer)];p=subprocess.run(cmd,capture_output=True,text=True);j['official'].append({'problem':problem,'input':inp.name,'input_sha256':sha(inp),'answer_sha256':sha(answer),'actual_sha256':sha(out),'checker_command':cmd,'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr});save();assert p.returncode==0,(cmd,p.stderr)
 print('official all passed',problem,flush=True)
j['headers_after']={str(p.relative_to(C)):sha(p) for p in sorted(C.rglob('*.hpp'))};j['headers_unchanged']=j['headers_before']==j['headers_after'];assert j['headers_unchanged'];j['harness_sha256']=sha(Path(__file__));save();print('final validation passed',flush=True)
