from pathlib import Path
from datetime import datetime,timezone
import concurrent.futures,fcntl,hashlib,json,subprocess,time
w=Path('/workspace/cp-fps-scratch-round3/mint-inline');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];commands={}
for v in ['base','candidate']:
 for problem,file in [('conv','convolution.test.cpp'),('exp','fps_exp.test.cpp'),('bostan','fps_linear_recurrence.test.cpp')]:commands[problem+'_'+v]=['g++',*flags,'-I'+str(w/v),str(w/v/'test'/file),'-o',str(w/(problem+'_'+v))]
 commands['traits_'+v]=['g++',*flags,'-Wall','-Wextra','-Werror','-I'+str(w/v),str(w/'traits_codegen.cpp'),'-o',str(w/('traits_'+v))]
for name,compiler in [('gcc13','/tmp/fps-gcc13/toolchain/usr/bin/g++-13'),('clang19','/tmp/fps-clang/toolchain/usr/bin/clang++-19')]:commands['traits_'+name]=[compiler,'-std=c++23','-O2','-Wall','-Wextra','-Werror','-I'+str(w/'candidate'),str(w/'traits_codegen.cpp'),'-o',str(w/('traits_'+name))]
j={'started_at':datetime.now(timezone.utc).isoformat(),'flags':flags,'compiler':subprocess.check_output(['g++','--version'],text=True),'snapshot_sha256':sha(w/'snapshot.json'),'harness_sha256':sha(Path(__file__)),'builds':{},'runs':{}}
def run(item):
 name,cmd=item;start=time.monotonic();r=subprocess.run(cmd,capture_output=True,text=True);return name,{'command':cmd,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'seconds':time.monotonic()-start,'source_sha256':sha(Path(cmd[-3])),'binary_sha256':sha(Path(cmd[-1])) if r.returncode==0 else None}
print('Waiting build SH',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 with concurrent.futures.ThreadPoolExecutor(2) as pool:
  for name,r in pool.map(run,commands.items()):j['builds'][name]=r;print(name,r['returncode'],round(r['seconds'],2),r['stderr'][:1200],flush=True)
 assert all(r['returncode']==0 for r in j['builds'].values())
 for name in ['traits_base','traits_candidate','traits_gcc13','traits_clang19']:
  cmd=[str(w/name)];r=subprocess.run(cmd,capture_output=True,text=True);j['runs'][name]={'command':cmd,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr};assert r.returncode==0,(name,r.stderr)
 assert len({r['stdout'] for r in j['runs'].values()})==1
 for v in ['base','candidate']:
  cmd=['g++',*flags,'-I'+str(w/v),'-S',str(w/'traits_codegen.cpp'),'-o',str(w/('traits_'+v+'.s'))];r=subprocess.run(cmd,capture_output=True,text=True);assert r.returncode==0,r.stderr;j['builds']['asm_'+v]={'command':cmd,'returncode':r.returncode,'assembly_sha256':sha(w/('traits_'+v+'.s'))}
  asm=subprocess.check_output(['objdump','-Cd',str(w/('traits_'+v))],text=True);(w/('traits_'+v+'.objdump')).write_text(asm);j['builds']['asm_'+v]['objdump_sha256']=sha(w/('traits_'+v+'.objdump'))
j['finished_at']=datetime.now(timezone.utc).isoformat();(w/'build.json').write_text(json.dumps(j,indent=2)+'\n');print('Builds/traits/codegen complete',flush=True)
