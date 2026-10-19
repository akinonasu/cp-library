from pathlib import Path
import fcntl,hashlib,json,subprocess
w=Path('/tmp/fps-avx512-top');d=Path('/workspace/cp-library/test/benchmark/results/fps_avx512/top');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();headers=lambda t:{str(p.relative_to(t)):sha(p)for p in sorted(t.rglob('*.hpp'))}
trees={v:w/v for v in ['wide','reordered_late2','window16']};j={'before':{v:headers(t)for v,t in trees.items()},'checks':{}}
commands={'window16':w/'verify_window16','late2_radix4':w/'verify_late2','wide_radix4':w/'verify_wide_final','wide_radix2':w/'verify2_wide'}
with open('/tmp/cp-fps-tuning.lock','a')as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 for name,p in commands.items():
  r=subprocess.run([str(p)],capture_output=True,text=True);j['checks'][name]={'command':[str(p)],'binary_sha256':sha(p),'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr};assert r.returncode==0,(name,r.stderr)
 j['after']={v:headers(t)for v,t in trees.items()};j['unchanged']=j['before']==j['after'];assert j['unchanged'];(d/'raw/final_checks.json').write_text(json.dumps(j,indent=2)+'\n');print({k:r['stdout'].strip()for k,r in j['checks'].items()})
