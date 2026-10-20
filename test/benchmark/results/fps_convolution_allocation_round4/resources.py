from pathlib import Path
import fcntl,hashlib,json,os,random,subprocess
W=Path('/workspace/cp-fps-round4/borrow');O=Path('/tmp/library-checker-problems/convolution/convolution_mod');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
os.sched_setaffinity(0,{2});r={'cpu':2,'seed':20261045,'script_sha256':sha(Path(__file__)),'samples':[]};rng=random.Random(r['seed'])
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX)
 for case in ['max_random_00','max_random_01','small_and_large_03']:
  for trial in range(3):
   variants=['before','capacity_1m'];rng.shuffle(variants)
   for variant in variants:
    binary=W/'capacity9'/variant;inp=O/'in'/(case+'.in');answer=O/'out'/(case+'.out');out=W/'resource.out'
    with inp.open('rb') as src,out.open('wb') as dst:
     child=subprocess.Popen([str(binary)],stdin=src,stdout=dst)
     pid,status,usage=os.wait4(child.pid,0);child.returncode=os.waitstatus_to_exitcode(status)
     assert child.returncode==0
    subprocess.run([str(O/'checker'),str(inp),str(out),str(answer)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    r['samples'].append({'case':case,'trial':trial,'variant':variant,'binary_sha256':sha(binary),'input_sha256':sha(inp),'max_rss_kib':usage.ru_maxrss,'minor_faults':usage.ru_minflt,'major_faults':usage.ru_majflt,'output_sha256':sha(out)})
 fcntl.flock(lock,fcntl.LOCK_UN)
(W/'resources.json').write_text(json.dumps(r,indent=2)+'\n');print('Resource diagnostics completed')
