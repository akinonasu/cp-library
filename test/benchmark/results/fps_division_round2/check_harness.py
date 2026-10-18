from pathlib import Path
import os,subprocess,time,json,hashlib,shutil
W=Path('/tmp/fps-division-round2');R=Path('/workspace/cp-library');C=W/'candidate';shutil.copyfile(R/'test/unit/fps_division_blocked.cpp',W/'unit_division.cpp');shutil.copyfile(R/'test/unit/polynomial_gcd.cpp',W/'unit_gcd.cpp')
header=W/'standalone.cpp';header.write_text('#include "components/fps_division_blocked.hpp"\n#include "components/fps_division_blocked.hpp"\n#if defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) || defined(all) || defined(sz) || defined(uint)\n#error "macro leakage"\n#endif\nint main() {}\n')
flags=['-std=c++23','-O2','-Wall','-Wextra','-Werror','-DDEBUG=1','-I',str(C)];result={'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'flags':flags,'headers':{str(p.relative_to(C)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(C.rglob('*.hpp'))},'sources':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in [header,W/'unit_division.cpp',W/'unit_gcd.cpp']},'commands':[]}
def run(name,cmd):
 print(name,flush=True);t=time.monotonic();env=os.environ.copy();env.update(ASAN_OPTIONS='detect_leaks=0',UBSAN_OPTIONS='halt_on_error=1');p=subprocess.run(cmd,capture_output=True,text=True,env=env);info={'name':name,'command':cmd,'seconds':time.monotonic()-t,'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr};result['commands'].append(info);(W/'check.json').write_text(json.dumps(result,indent=2)+'\n');print(info['returncode'],info['seconds'],p.stdout,p.stderr,flush=True);assert p.returncode==0
run('standalone',['g++',*flags,'-fsyntax-only',str(header)])
for unit in ['division','gcd']:
 binary=W/('unit_'+unit+'_normal');run(unit+'_compile',['g++',*flags,str(W/('unit_'+unit+'.cpp')),'-o',str(binary)]);run(unit+'_run',[str(binary)])
binary=W/'unit_division_sanitize';run('division_sanitize_compile',['g++',*flags,'-fsanitize=address,undefined','-fno-omit-frame-pointer','-g',str(W/'unit_division.cpp'),'-o',str(binary)]);run('division_sanitize_run',[str(binary)])
print('all checks passed',flush=True)
