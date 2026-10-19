from pathlib import Path
import argparse,fcntl,hashlib,io,json,shutil,subprocess,tarfile

P=argparse.ArgumentParser()
P.add_argument('--repository',type=Path,default=Path(__file__).resolve().parents[4])
P.add_argument('--work',type=Path,default=Path('/tmp/fps-division-round3-replay'))
P.add_argument('--official-root',type=Path,default=Path('/tmp/library-checker-problems'))
P.add_argument('--references',type=Path,default=Path('/tmp/fps-reference'))
P.add_argument('--prepare-only',action='store_true')
P.add_argument('--diagnostics',action='store_true')
a=P.parse_args();a.repository=a.repository.resolve();a.work=a.work.resolve();a.official_root=a.official_root.resolve();a.references=a.references.resolve()
D=Path(__file__).resolve().parent;W=a.work;sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();baseline='bd951dce6fdcb3ea9db4fcef342147012a8ab0c0'
assert not W.exists(),'Choose a new replay directory'
W.mkdir(parents=True)
archive=subprocess.check_output(['git','-C',str(a.repository),'archive',baseline])
(W/'base').mkdir()
with tarfile.open(fileobj=io.BytesIO(archive),mode='r:') as t:t.extractall(W/'base',filter='data')
for variant,patch in [('candidate','divisor_candidate.patch'),('bostan_candidate','bostan_candidate.patch')]:
 shutil.copytree(W/'base',W/variant)
 subprocess.run(['git','apply',str(D/patch)],cwd=W/variant,check=True)
for name in ['driver.cpp','plan_driver.cpp','batch_driver.cpp','bostan_unit.cpp']:
 shutil.copy2(D/name,W/name)
records={}
for name in ['build.json','bostan_build.json']:
 record=json.loads((D/name).read_text());records[name]=record
 for variant,expected in record['headers'].items():
  actual={str(p.relative_to(W/variant)):sha(p) for p in sorted((W/variant).rglob('*.hpp'))}
  assert actual==expected,(variant,'header snapshot mismatch')
refs={}
for problem,id,record in [('division_of_polynomials','409561',records['build.json']),('kth_term_of_linearly_recurrent_sequence','405848',records['bostan_build.json'])]:
 source=a.references/problem/(id+'.cpp');actual=sha(source);assert actual==record['commands']['reference']['source_sha256'],(problem,'reference changed');refs[problem]={'path':str(source),'sha256':actual}
prepared={'baseline':baseline,'archive_sha256':hashlib.sha256(archive).hexdigest(),'source_snapshots_verified':True,'references':refs,'patches':{name:sha(D/name) for name in ['divisor_candidate.patch','bostan_candidate.patch']},'drivers':{name:sha(W/name) for name in ['driver.cpp','plan_driver.cpp','batch_driver.cpp','bostan_unit.cpp']},'prepare_only':a.prepare_only,'diagnostics':a.diagnostics}
(W/'replay_preparation.json').write_text(json.dumps(prepared,indent=2)+'\n')
print('Header snapshots and unedited references verified',flush=True)
if a.prepare_only:raise SystemExit
if a.diagnostics:
 flags=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell']
 builds={}
 with open('/tmp/cp-fps-tuning.lock','a') as lock:
  fcntl.flock(lock,fcntl.LOCK_SH)
  for label in ['plan','batch']:
   command=['g++',*flags,'-I'+str(W/'base'),str(W/(label+'_driver.cpp')),'-o',str(W/(label+'_driver'))]
   result=subprocess.run(command,capture_output=True,text=True)
   builds[label]={'command':command,'returncode':result.returncode,'stdout':result.stdout,'stderr':result.stderr,'source_sha256':sha(W/(label+'_driver.cpp')),'binary_sha256':sha(W/(label+'_driver')) if result.returncode==0 else None}
   assert result.returncode==0,result.stderr
 (W/'diagnostic_build.json').write_text(json.dumps(builds,indent=2)+'\n')
 for name in ['plan_measure.py','batch_measure.py']:
  source=(D/name).read_text().replace('/tmp/fps-division-round3',str(W)).replace('/tmp/library-checker-problems',str(a.official_root))
  (W/name).write_text(source)
  subprocess.run(['python3',str(W/name)],check=True)
 raise SystemExit
for name in ['build.py','measure.py','bostan_build.py','bostan_measure.py']:
 source=(D/name).read_text().replace('/tmp/fps-division-round3',str(W)).replace('/tmp/fps-reference',str(a.references)).replace('/tmp/library-checker-problems',str(a.official_root))
 (W/name).write_text(source)
for command in [['build.py'],['bostan_build.py'],['measure.py'],['measure.py','--seed','20261031','--output','divisor_repeat_raw.json'],['bostan_measure.py'],['bostan_measure.py','--seed','20261102','--output','bostan_repeat_raw.json']]:
 subprocess.run(['python3',str(W/command[0]),*command[1:]],cwd=a.repository,check=True)
