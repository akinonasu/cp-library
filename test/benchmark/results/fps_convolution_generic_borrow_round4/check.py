from pathlib import Path
import fcntl,hashlib,json,os,subprocess
w=Path(__file__).resolve().parent;root=Path('/workspace/cp-fps-round4/root/fallback_borrow');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
source=w/'fps_convolution_borrow.cpp';header=root/'components/fps_convolution.hpp'
meta={'header_sha256':sha(header),'unit_sha256':sha(source),'dependency_hashes':{},'checks':[]}
for command in [['g++','-std=c++23','-O2','-Wall','-Wextra','-Werror'],['g++','-std=c++23','-O2','-Wall','-Wextra','-fsanitize=address,undefined','-fno-omit-frame-pointer','-g'],['/tmp/fps-gcc13/toolchain/usr/bin/g++-13','-std=c++23','-O2','-Wall','-Wextra','-Werror']]:
 name='gcc14_sanitize' if '-g' in command else 'gcc13' if 'g++-13' in command[0] else 'gcc14'
 binary=w/name;cmd=[*command,'-I'+str(root),str(source),'-o',str(binary)]
 with open('/tmp/cp-fps-tuning.lock','w') as lock,(w/(name+'.log')).open('w') as log:
  fcntl.flock(lock,fcntl.LOCK_SH)
  subprocess.run(cmd,check=True,stdout=log,stderr=log)
  subprocess.run([str(binary)],check=True,stdout=log,stderr=log,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1'))
  fcntl.flock(lock,fcntl.LOCK_UN)
 meta['checks'].append({'name':name,'command':cmd,'binary_sha256':sha(binary),'passed':True})
 (w/'checks.json').write_text(json.dumps(meta,indent=2)+'\n')
assert sha(header)==meta['header_sha256'] and sha(source)==meta['unit_sha256']
print('three checks passed')
