from pathlib import Path
import fcntl,hashlib,json,os,subprocess
w=Path(__file__).resolve().parent;root=w/'candidate_noinline';source=w/'unit.cpp';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();meta={'unit_sha256':sha(source),'header_sha256':sha(root/'components/fps_convolution_constant.hpp'),'integration_sha256':sha(root/'components/fps_convolution.hpp'),'checks':[]}
checks=[('gcc14',['g++','-std=c++23','-O2','-Wall','-Wextra','-Werror'],source),('clang19',['/tmp/fps-clang/toolchain/usr/bin/clang++-19','-std=c++23','-O2','-Wall','-Wextra','-Werror'],source),('gcc14_sanitize',['g++','-std=c++23','-O2','-Wall','-Wextra','-fsanitize=address,undefined','-fno-omit-frame-pointer','-g'],source),('gcc13',['/tmp/fps-gcc13/toolchain/usr/bin/g++-13','-std=c++23','-O2','-Wall','-Wextra','-Werror'],source),('gcc14_nonlinux_standalone',['g++','-std=c++23','-O2','-Wall','-Wextra','-Werror','-U__linux__'],w/'standalone.cpp'),('gcc13_standalone',['/tmp/fps-gcc13/toolchain/usr/bin/g++-13','-std=c++23','-O2','-Wall','-Wextra','-Werror'],w/'standalone.cpp'),('clang19_standalone',['/tmp/fps-clang/toolchain/usr/bin/clang++-19','-std=c++23','-O2','-Wall','-Wextra','-Werror'],w/'standalone.cpp')]
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 for name,cmd,src in checks:
  binary=w/('final_'+name);command=[*cmd,'-I'+str(root),str(src),'-o',str(binary)]
  with (w/('final_'+name+'.log')).open('w') as log:
   subprocess.run(command,check=True,stdout=log,stderr=log)
   subprocess.run([str(binary)],check=True,stdout=log,stderr=log,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  meta['checks'].append({'name':name,'command':command,'binary_sha256':sha(binary),'passed':True});(w/'final_checks.json').write_text(json.dumps(meta,indent=2)+'\n');print(name,'passed',flush=True)
 fcntl.flock(lock,fcntl.LOCK_UN)
assert sha(source)==meta['unit_sha256'] and sha(root/'components/fps_convolution_constant.hpp')==meta['header_sha256'] and sha(root/'components/fps_convolution.hpp')==meta['integration_sha256']
