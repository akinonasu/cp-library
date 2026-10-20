import argparse,fcntl,hashlib,json,os,random,statistics,subprocess
from pathlib import Path
WORK=Path(__file__).resolve().parent
PROBLEMS={'convolution_mod':'convolution/convolution_mod','log_of_formal_power_series':'polynomial/log_of_formal_power_series'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--trials',type=int,default=9);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 variants=['baseline','input_extra_capacity','reference'];cases=['convolution_mod:max_random_00','convolution_mod:small_and_large_01','log_of_formal_power_series:max_random_00'];launcher=WORK/'memory_launcher'
 result={'purpose':'Separate physical memory pass, not a formal whole-process timing comparison. Small native launcher forks the unedited binary, so child ru_maxrss is not floored by the Python controller RSS retained across exec. Each sample is the target grandchild wait4 usage.','trials':a.trials,'cpu':2,'lock':'EX /tmp/cp-fps-tuning.lock','harness_sha256':sha(Path(__file__)),'launcher_source_sha256':sha(WORK/'memory_launcher.c'),'launcher_binary_sha256':sha(launcher),'binaries':{p+'_'+v:sha(WORK/(p+'_'+v)) for p in PROBLEMS for v in variants},'results':[]}
 with open('/tmp/cp-fps-tuning.lock','a') as lock:
  fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261022)
  for spec in cases:
   problem,case=spec.split(':');folder=Path('/tmp/library-checker-problems')/PROBLEMS[problem];inp=folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out');checker=folder/'checker';row={'problem':problem,'case':case,'input_sha256':sha(inp),'answer_sha256':sha(answer),'checker_sha256':sha(checker),'samples':[],'variants':{}}
   for trial in range(a.trials+1):
    order=variants[:];rng.shuffle(order)
    for v in order:
     binary=WORK/(problem+'_'+v);output=WORK/('memory_'+problem+'_'+case+'_'+v+'.out')
     with inp.open('rb') as fin,output.open('wb') as fout:r=subprocess.run([str(launcher),str(binary)],stdin=fin,stdout=fout,stderr=subprocess.PIPE,text=True,check=True)
     usage=json.loads(r.stderr.strip());subprocess.run([str(checker),str(inp),str(output),str(answer)],capture_output=True,check=True)
     row['samples'].append({'trial':trial,'warmup':trial==0,'variant':v,'order':order,'usage':usage,'checker_pass':True,'output_sha256':sha(output)})
   for v in variants:
    samples=[s['usage'] for s in row['samples'] if not s['warmup'] and s['variant']==v]
    row['variants'][v]={'median':{k:statistics.median(s[k] for s in samples) for k in ['maxrss_kib','minflt','majflt']},'max':{k:max(s[k] for s in samples) for k in ['maxrss_kib','minflt','majflt']},'output_path':str(WORK/('memory_'+problem+'_'+case+'_'+v+'.out'))}
   result['results'].append(row);a.output.write_text(json.dumps(result,indent=2)+'\n');print(spec,{v:row['variants'][v]['median'] for v in variants},flush=True)
if __name__=='__main__':main()
