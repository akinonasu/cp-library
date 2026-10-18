from pathlib import Path
import os,fcntl,subprocess,time,json,random,statistics,hashlib,platform
W=Path('/tmp/fps-division');R=Path('/tmp/library-checker-problems/polynomial');ROOT=W/'current';rng=random.Random(20261019)

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def run(binary,inp,out):
 with inp.open('rb') as f,out.open('wb') as g:
  start=time.perf_counter_ns();p=subprocess.run([str(W/binary)],stdin=f,stdout=g,stderr=subprocess.PIPE,timeout=120);end=time.perf_counter_ns()
 assert p.returncode==0,(binary,inp,p.stderr)
 return (end-start)/1e6,(float(p.stderr) if p.stderr else None)

cases=[]
for fname in ['v_random_00','v_random_01','v_random_02','v_random_03','u_eq_0_00']:
 cases.append(('division_'+fname,R/'division_of_polynomials/in'/(fname+'.in'),['division_baseline','division_current','division_reference'],'division_of_polynomials'))
cases.append(('inverse_max',R/'inv_of_polynomials/in/max_random_00.in',['inverse_mod_baseline','inverse_mod_current','inverse_mod_reference'],'inv_of_polynomials'))
cases.append(('gcd_max',R/'inv_of_polynomials/in/max_random_00.in',['gcd_baseline','gcd_current'],None))
series=W/'series.in'
if not series.exists():
 n=500000;series.write_text(str(n)+' '+str(n)+'\n'+' '.join(str(rng.randrange(998244353)) for _ in range(n))+'\n1 '+' '.join(str(rng.randrange(998244353)) for _ in range(n-1))+'\n')
cases.append(('series_max',series,['series_baseline','series_current'],None))
os.sched_setaffinity(0,{2})
lock=open('/tmp/cp-fps-tuning.lock','a');fcntl.flock(lock,fcntl.LOCK_EX)
result={'baseline':'ce81108','flags':json.loads((W/'commands.json').read_text()),'cpu':Path('/proc/cpuinfo').read_text().split('model name')[1].splitlines()[0],'platform':platform.platform(),'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'cpu_affinity':[2],'seed':20261019,'trials':9,'metric':'whole process wall milliseconds; computation with allocations reported separately by driver','driver_sha256':sha(W/'driver.cpp'),'harness_sha256':sha(Path(__file__)),'headers':{str(p.relative_to(ROOT)):sha(p) for folder in ['fps','components','math'] for p in sorted((ROOT/folder).glob('*.hpp'))},'reference_sha256':{p.name:sha(p) for p in [Path('/tmp/fps-reference/division_of_polynomials/409561.cpp'),Path('/tmp/fps-reference/inv_of_polynomials/409589.cpp')]},'workloads':{}}
for name,inp,binaries,problem in cases:
 samples={b:[] for b in binaries};kernel={b:[] for b in binaries};expected=None
 def verify(binary,out):
  global expected
  if problem:
   root=R/problem;c=subprocess.run([str(root/'checker'),str(inp),str(out),str(root/'out'/(inp.stem+'.out'))],capture_output=True);assert c.returncode==0,(name,binary,c.stderr)
  else:
   tokens=b' '.join(out.read_bytes().split())
   if expected is None:expected=tokens
   assert tokens==expected,(name,binary)
 for b in binaries:
  out=W/(b+'.out');run(b,inp,out);verify(b,out)
 order=binaries*9;rng.shuffle(order)
 for b in order:
  out=W/(b+'.out');ms,km=run(b,inp,out);samples[b].append(ms)
  if km is not None:kernel[b].append(km)
  verify(b,out)
 item={'input':str(inp),'input_header':inp.open().readline().strip(),'input_sha256':sha(inp),'binaries':{b:{'binary_sha256':sha(W/b),'whole_ms':samples[b],'median_ms':statistics.median(samples[b]),'computation_ms':kernel[b],'compute_median_ms':statistics.median(kernel[b]) if kernel[b] else None} for b in binaries}}
 result['workloads'][name]=item
 print(name,{b:{'whole':round(v['median_ms'],3),'kernel':round(v['compute_median_ms'],3) if v['compute_median_ms'] is not None else None} for b,v in item['binaries'].items()},flush=True)
(W/'result.json').write_text(json.dumps(result,indent=2)+'\n')
