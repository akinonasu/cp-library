import argparse,concurrent.futures,fcntl,hashlib,json,os,platform,random,statistics,subprocess,time
from pathlib import Path
WORK=Path(__file__).resolve().parent
FLAGS=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
PROBLEMS={'convolution_mod':('convolution/convolution_mod','test/convolution.test.cpp',409465),'log_of_formal_power_series':('polynomial/log_of_formal_power_series','test/fps_log.test.cpp',409452)}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--variants',nargs='+',default=['baseline','parser_montgomery']);p.add_argument('--trials',type=int,default=9);p.add_argument('--output',type=Path,required=True);p.add_argument('--seed',type=int,default=20261020);p.add_argument('--no-build',action='store_true');p.add_argument('--rusage',action='store_true');p.add_argument('--cases',nargs='+',default=['convolution_mod:max_random_00','convolution_mod:max_random_01','convolution_mod:small_and_large_01','log_of_formal_power_series:max_random_00','log_of_formal_power_series:max_random_01']);a=p.parse_args()
 problems=sorted(set(x.split(':')[0] for x in a.cases));variants=[*a.variants,'reference']
 result={'baseline':'7521ea8703a362aaf3ccd370188fccab86050848','flags':FLAGS,'cpu':2,'trials':a.trials,'seed':a.seed,'harness_sha256':sha(Path(__file__)),'compiler':subprocess.check_output(['g++','--version'],text=True).splitlines()[0],'hardware':subprocess.check_output(['lscpu'],text=True),'rusage_enabled':a.rusage,'timing_wait_api':'subprocess.Popen then os.wait4' if a.rusage else 'subprocess.run','thp_settings':{name:Path('/sys/kernel/mm/transparent_hugepage/'+name).read_text().strip() for name in ['enabled','defrag'] if Path('/sys/kernel/mm/transparent_hugepage/'+name).is_file()},'official_commit':subprocess.check_output(['git','-C','/tmp/library-checker-problems','rev-parse','HEAD'],text=True).strip(),'sources':{},'results':[],'scope':'Whole uninstrumented process with regular-file stdin/stdout. Original references are unedited, including their preinit/_exit. One warmup and shuffled paired trials. Checker input actual answer after every run.'}
 jobs=[]
 for variant in variants:
  for problem in problems:
   info=PROBLEMS[problem];src=Path('/tmp/fps-reference')/problem/(str(info[2])+'.cpp') if variant=='reference' else WORK/variant/info[1]
   binary=WORK/(problem+'_'+variant);command=['g++',*FLAGS]
   if variant!='reference':command+=['-I',str(WORK/variant)]
   command += [str(src),'-o',str(binary)]
   dep={str(q.relative_to(WORK/variant)):sha(q) for folder in ('components','math','fps','convolution') for q in (WORK/variant/folder).glob('*.hpp')} if variant!='reference' else {}
   if variant!='reference':dep['test/fps_io.hpp']=sha(WORK/variant/'test/fps_io.hpp')
   jobs.append((variant,problem,src,binary,command,dep))
 def build(job):
  variant,problem,src,binary,command,dep=job
  if not a.no_build:subprocess.run(command,check=True)
  return problem+'_'+variant,{'source_path':str(src),'source_sha256':sha(src),'binary_path':str(binary),'binary_sha256':sha(binary),'command':command,'dependencies':dep}
 with open('/tmp/cp-fps-tuning.lock','a') as lock:
  fcntl.flock(lock,fcntl.LOCK_SH)
  with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
   for key,value in pool.map(build,jobs):result['sources'][key]=value;print('built',key,flush=True)
  fcntl.flock(lock,fcntl.LOCK_UN)
  print('waiting EX',flush=True);fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});print('timing',flush=True)
  rng=random.Random(a.seed)
  for spec in a.cases:
   problem,case=spec.split(':');folder=Path('/tmp/library-checker-problems')/PROBLEMS[problem][0];inp=folder/'in'/(case+'.in');answer=folder/'out'/(case+'.out');checker=folder/'checker'
   row={'problem':problem,'case':case,'input_sha256':sha(inp),'answer_sha256':sha(answer),'checker_sha256':sha(checker),'samples':[],'variants':{}}
   samples={v:[] for v in variants}
   for trial in range(a.trials+1):
    order=variants[:];rng.shuffle(order)
    for v in order:
     binary=WORK/(problem+'_'+v);output=WORK/(problem+'_'+case+'_'+v+'.out')
     with inp.open('rb') as fin,output.open('wb') as fout:
      begin=time.perf_counter_ns()
      if a.rusage:
       process=subprocess.Popen([str(binary)],stdin=fin,stdout=fout)
       _,status,usage=os.wait4(process.pid,0);process.returncode=os.waitstatus_to_exitcode(status)
       if process.returncode:raise subprocess.CalledProcessError(process.returncode,[str(binary)])
      else:
       subprocess.run([str(binary)],stdin=fin,stdout=fout,check=True);usage=None
      ns=time.perf_counter_ns()-begin
     checked=subprocess.run([str(checker),str(inp),str(output),str(answer)],capture_output=True,text=True)
     if checked.returncode:raise RuntimeError((spec,v,checked.stderr))
     row['samples'].append({'trial':trial,'warmup':trial==0,'variant':v,'order':order,'ns':ns,'checker_pass':True,'output_sha256':sha(output),'rusage':{'maxrss_kib':usage.ru_maxrss,'minflt':usage.ru_minflt,'majflt':usage.ru_majflt} if usage else None})
     if trial:samples[v].append(ns/1e6)
   for v,measurements in samples.items():
    output=WORK/(problem+'_'+case+'_'+v+'.out');row['variants'][v]={'rusage_median':{key:statistics.median(s['rusage'][key] for s in row['samples'] if not s['warmup'] and s['variant']==v) for key in ['maxrss_kib','minflt','majflt']} if a.rusage else None,'median_ms':statistics.median(measurements),'samples_ms':measurements,'output_path':str(output),'output_sha256':sha(output)}
   result['results'].append(row);a.output.write_text(json.dumps(result,indent=2)+'\n');print(spec,{v:x['median_ms'] for v,x in row['variants'].items()},flush=True)
if __name__=='__main__':main()
