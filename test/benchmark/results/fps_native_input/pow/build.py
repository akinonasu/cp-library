from pathlib import Path
import fcntl,hashlib,json,subprocess
W=Path('/tmp/fps-pow-native-probe');flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
result={'flags':flags,'cpu':2,'assertions':True,'trials':21,'baseline':'4765ceb','reference_submission':409718,'sources':{},'kernel_notes':'same rng(20261019), leading coefficient 1, exponent 10^18; fps public const pow and reference poly::power from distinct const canonical uint32 input; timer includes transform/arena and output allocation, excludes input generation/hash'}
lock=open('/tmp/cp-fps-tuning.lock','w');fcntl.flock(lock,fcntl.LOCK_SH)
for v in ['baseline','candidate','reference']:
 for kind in ['kernel','whole']:
  root=W/v
  source=(W/'reference_kernel.cpp' if kind=='kernel' else Path('/tmp/fps-reference/pow_of_formal_power_series/409718.cpp')) if v=='reference' else (W/'kernel.cpp' if kind=='kernel' else root/'test/fps_pow.test.cpp')
  binary=W/(v+'_'+kind);cmd=['g++',*flags,'-I',str(root),str(source),'-o',str(binary)]
  subprocess.run(cmd,check=True)
  result['sources'][v+'_'+kind]={'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'command':cmd}
  print('built',v,kind,flush=True)
 if v!='reference':result['sources'][v+'_headers']={str(p.relative_to(root)):hashlib.sha256(p.read_bytes()).hexdigest() for p in root.rglob('*.hpp')}
for unit in ['fps_exp_blocked','fps_algorithms']:
 cmd=['g++',*flags,'-I',str(W/'candidate'),str(W/'candidate/test/unit'/f'{unit}.cpp'),'-o',str(W/unit)]
 subprocess.run(cmd,check=True);subprocess.run([str(W/unit)],check=True);print('passed',unit,flush=True)
result['compiler']=subprocess.check_output(['g++','--version'],text=True);result['cpu_model']=subprocess.check_output(['lscpu'],text=True)
(W/'metadata.json').write_text(json.dumps(result,indent=2)+'\n')
