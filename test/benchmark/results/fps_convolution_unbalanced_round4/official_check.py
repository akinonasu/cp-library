from pathlib import Path
import fcntl,hashlib,json,subprocess
w=Path(__file__).resolve().parent;off=Path('/workspace/cp-fps-scratch-round3/library-checker-problems/convolution/convolution_mod')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
result={'binary_sha256':sha(w/'current'),'checker_sha256':sha(off/'checker'),'harness_sha256':sha(Path(__file__)),'cases':[]}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 for inp in sorted((off/'in').glob('*.in')):
  out=w/'official.out';ans=off/'out'/(inp.stem+'.out')
  with inp.open('rb') as a,out.open('wb') as b:subprocess.run([str(w/'current')],stdin=a,stdout=b,check=True)
  cp=subprocess.run([str(off/'checker'),str(inp),str(out),str(ans)],capture_output=True,text=True)
  result['cases'].append({'case':inp.stem,'input_sha256':sha(inp),'answer_sha256':sha(ans),'output_sha256':sha(out),'returncode':cp.returncode,'stderr':cp.stderr})
  if cp.returncode:raise RuntimeError(result['cases'][-1])
 fcntl.flock(lock,fcntl.LOCK_UN)
(w/'official_check.json').write_text(json.dumps(result,indent=2)+'\n')
print('passed',len(result['cases']))
