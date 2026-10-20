from pathlib import Path
import fcntl,hashlib,json,os,subprocess,time
W=Path('/workspace/cp-fps-round4/borrow');C=W/'capacity_1m';O=Path('/tmp/library-checker-problems/convolution/convolution_mod');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def headers():return {str(p.relative_to(C)):sha(p) for f in ['components','fps','math','convolution','test'] for p in sorted((C/f).rglob('*.hpp'))}
r={'candidate':'capacity_1m','baseline':'7521ea8703a362aaf3ccd370188fccab86050848','harness_sha256':sha(Path(__file__)),'headers_before':headers(),'compiler':subprocess.check_output(['g++','--version'],text=True),'checks':[]}
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 binary=W/'capacity9/capacity_1m';count=0
 for inp in sorted((O/'in').glob('*.in')):
  output=W/'validation.out';answer=O/'out'/(inp.stem+'.out')
  with inp.open('rb') as src,output.open('wb') as dst:subprocess.run([str(binary)],stdin=src,stdout=dst,check=True)
  command=[str(O/'checker'),str(inp),str(output),str(answer)];subprocess.run(command,check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  r['checks'].append({'input':inp.name,'input_sha256':sha(inp),'answer_sha256':sha(answer),'output_sha256':sha(output),'checker':command,'passed':True});count+=1
 fcntl.flock(lock,fcntl.LOCK_UN)
 r['official_cases']=count;r['official_binary_sha256']=sha(binary);r['units']=[]
 for name,extra in [('normal',['-Werror']),('sanitize',['-fsanitize=address,undefined','-fno-omit-frame-pointer','-g'])]:
  source=C/'test/unit/fps_convolution_borrow.cpp';binary=W/('capacity_'+name);command=['g++','-std=c++23','-O2','-Wall','-Wextra',*extra,'-I'+str(C),str(source),'-o',str(binary)]
  env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1')
  fcntl.flock(lock,fcntl.LOCK_SH)
  with (W/('capacity_'+name+'.log')).open('w') as log:
   subprocess.run(command,check=True,stdout=log,stderr=log)
   subprocess.run([str(binary)],check=True,stdout=log,stderr=log,env=env)
  fcntl.flock(lock,fcntl.LOCK_UN)
  r['units'].append({'name':name,'command':command,'source_sha256':sha(source),'binary_sha256':sha(binary),'environment':{k:env[k] for k in ['ASAN_OPTIONS','UBSAN_OPTIONS']},'passed':True})
r['headers_after']=headers();assert r['headers_before']==r['headers_after'];r['passed']=True
(W/'capacity_validation.json').write_text(json.dumps(r,indent=2)+'\n');print('53 official cases and normal/San unit passed')
