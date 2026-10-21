import subprocess,json,fcntl,os,statistics,time,sys
from pathlib import Path
p=Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path('/workspace/cp-fps-256-fallback-study')
locks=[open('/tmp/cp-fps-tuning.lock','a'),open('/tmp/cp-wm-opt.lock','a')]
for f in locks:fcntl.flock(f,fcntl.LOCK_EX)
os.sched_setaffinity(0,{min(os.sched_getaffinity(0))})
trials=[]
for t in range(3):
 row={}
 for v in (['log_after','inverse_generic'] if t%2==0 else ['inverse_generic','log_after']):
  r=subprocess.run([str(p/v/'gcc'),'bench'],text=True,capture_output=True,check=True)
  row[v]={'samples':[[int(n),int(m),float(ns)] for n,m,ns in (line.split() for line in r.stdout.splitlines())],'checksum':r.stderr.strip()}
 trials.append(row)
summary=[]
for i in range(len(trials[0]['log_after']['samples'])):
 old=statistics.median(x['log_after']['samples'][i][2] for x in trials);new=statistics.median(x['inverse_generic']['samples'][i][2] for x in trials)
 n,m=trials[0]['log_after']['samples'][i][:2];summary.append({'precision':n,'input':m,'before_ns':old,'after_ns':new,'ratio':new/old})
data={'date_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'cpu_affinity':sorted(os.sched_getaffinity(0)),'cpu':'AMD EPYC 9V74','compiler':subprocess.check_output([sys.argv[2] if len(sys.argv) > 2 else 'g++','--version'],text=True).splitlines()[0],'flags':'-std=c++20 -O2 -DNDEBUG','trials':trials,'medians':summary}
(p/'inverse_benchmark.json').write_text(json.dumps(data,indent=2)+'\n')
for v in summary:print(v,flush=True)
