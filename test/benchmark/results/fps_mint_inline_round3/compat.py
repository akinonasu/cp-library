from pathlib import Path
from datetime import datetime,timezone
import concurrent.futures,difflib,fcntl,hashlib,json,re,subprocess
w=Path('/workspace/cp-fps-scratch-round3/mint-inline');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();j={'started_at':datetime.now(timezone.utc).isoformat(),'harness_sha256':sha(Path(__file__)),'builds':{},'runs':{},'codegen':{}}
commands={name:[compiler,'-std=c++23','-O0','-Wall','-Wextra','-Werror','-I'+str(w/'candidate'),str(w/'traits_codegen.cpp'),'-o',str(w/('traits_o0_'+name))] for name,compiler in [('gcc14','g++'),('gcc13','/tmp/fps-gcc13/toolchain/usr/bin/g++-13'),('clang19','/tmp/fps-clang/toolchain/usr/bin/clang++-19')]}
def build(item):
 name,cmd=item;r=subprocess.run(cmd,capture_output=True,text=True);return name,{'command':cmd,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'binary_sha256':sha(Path(cmd[-1])) if r.returncode==0 else None}
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 with concurrent.futures.ThreadPoolExecutor(2) as pool:
  for name,r in pool.map(build,commands.items()):j['builds'][name]=r;assert r['returncode']==0,r['stderr']
 for name,record in j['builds'].items():
  r=subprocess.run([record['command'][-1]],capture_output=True,text=True);j['runs'][name]={'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr};assert r.returncode==0,r['stderr']
for v in ['base','candidate']:
 asm=(w/('traits_'+v+'.s')).read_text()
 def body(symbol):
  start=asm.index('\n'+symbol+':')
  return asm[start:asm.index('\n\t.size\t'+symbol+',',start)]
 b=body('_Z11make_vectorm');single=body('_Z12make_defaultv')
 j['codegen'][v]={'assembly_sha256':sha(w/('traits_'+v+'.s')),'all_ctor_call_sites':len(re.findall(r'\bcall\s+_ZN10fps_detail4MintC[12]Ev',asm)),'all_memset_call_sites':len(re.findall(r'\bcall\s+memset',asm)),'vector_ctor_call_sites':len(re.findall(r'\bcall\s+_ZN10fps_detail4MintC[12]Ev',b)),'vector_memset_call_sites':len(re.findall(r'\bcall\s+memset',b)),'vector_body':b,'single_default_body':single}
p=w/'candidate.patch';p.write_text(''.join(difflib.unified_diff((w/'base/math/mint.hpp').read_text().splitlines(True),(w/'candidate/math/mint.hpp').read_text().splitlines(True),fromfile='a/math/mint.hpp',tofile='b/math/mint.hpp')))
j['production_mint_unchanged']=sha(Path('/workspace/cp-library/math/mint.hpp'))==sha(w/'base/math/mint.hpp');assert j['production_mint_unchanged'];j['finished_at']=datetime.now(timezone.utc).isoformat();(w/'compat.json').write_text(json.dumps(j,indent=2)+'\n');print('O0 GCC14/GCC13/Clang19 passed; codegen',[(v,j['codegen'][v]['all_ctor_call_sites'],j['codegen'][v]['vector_memset_call_sites']) for v in ['base','candidate']],flush=True)
