from pathlib import Path
import fcntl,hashlib,json,os,shlex,shutil,subprocess
w=Path(__file__).resolve().parent;root=w/'candidate_noinline';source=w/'unit.cpp';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def compiler_info(cmd):
 p=Path(shutil.which(cmd) or cmd).resolve();front=subprocess.check_output([str(p),'-print-prog-name=cc1plus'],text=True).strip();q=Path(front)
 return {'invocation':cmd,'resolved_executable':str(p),'sha256':sha(p),'version':subprocess.check_output([str(p),'--version'],text=True),'cc1plus':{'path':str(q),'sha256':sha(q)} if q.is_file() else front}
def deps(compiler,flags,src,name):
 command=[compiler,*flags,'-MM','-MT','unit',str(src)];raw=subprocess.check_output(command,text=True);(w/(name+'.d')).write_text(raw);paths=shlex.split(raw.split(':',1)[1].replace('\\\n',' '));records=[]
 for x in paths:
  p=Path(x).resolve();records.append({'path':str(p),'tree_relative':str(p.relative_to(root)) if p.is_relative_to(root) else None,'source':p==source.resolve(),'sha256':sha(p)})
 return {'command':command,'raw_dependency_file':name+'.d','raw_dependency_sha256':sha(w/(name+'.d')),'files':records}
compiler='g++';base=['-std=c++2b','-O2','-Wall','-Wextra','-DDEBUG=1','-I',str(root)];result={'unit_sha256':sha(source),'compiler':compiler_info(compiler),'checks':[]}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 for name,flags in [('normal',base),('sanitize',base+['-fsanitize=address,undefined','-fno-omit-frame-pointer','-g'])]:
  before=deps(compiler,flags,source,'reuse_'+name+'_before');binary=w/('reuse_'+name);command=[compiler,*flags,str(source),'-o',str(binary)];log=w/('reuse_'+name+'.log')
  with log.open('w') as f:
   subprocess.run(command,check=True,stdout=f,stderr=f);subprocess.run([str(binary)],check=True,stdout=f,stderr=f,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  after=deps(compiler,flags,source,'reuse_'+name+'_after');assert before['files']==after['files']
  result['checks'].append({'name':name,'flags':flags,'command':command,'dependencies_before':before,'dependencies_after':after,'binary_sha256':sha(binary),'log_file':log.name,'log_sha256':sha(log),'run_env':{'ASAN_OPTIONS':'detect_leaks=0','UBSAN_OPTIONS':'halt_on_error=1'},'passed':True});(w/'reuse_checks.json').write_text(json.dumps(result,indent=2)+'\n');print(name,'passed',flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
assert sha(source)==result['unit_sha256']
