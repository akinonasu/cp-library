from pathlib import Path
import argparse,concurrent.futures,fcntl,hashlib,io,json,shutil,subprocess,tarfile

p=argparse.ArgumentParser()
p.add_argument('--repository',type=Path,default=Path(__file__).resolve().parents[4])
p.add_argument('--work',type=Path,default=Path('/workspace/cp-fps-scratch-round3/mint-inline-replay'))
p.add_argument('--official-root',type=Path,default=Path('/tmp/library-checker-problems'))
p.add_argument('--compiler',default='g++')
p.add_argument('--prepare-only',action='store_true')
a=p.parse_args();a.repository=a.repository.resolve();a.work=a.work.resolve();a.official_root=a.official_root.resolve();d=Path(__file__).resolve().parent;w=a.work;sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();baseline='bd951dce6fdcb3ea9db4fcef342147012a8ab0c0'
assert not w.exists(),'Choose a new work directory'
w.mkdir(parents=True);archive=subprocess.check_output(['git','-C',str(a.repository),'archive',baseline]);(w/'base').mkdir()
with tarfile.open(fileobj=io.BytesIO(archive),mode='r:') as t:t.extractall(w/'base',filter='data')
subprocess.run(['git','apply',str(d/'snapshot_base.patch')],cwd=w/'base',check=True)
shutil.copytree(w/'base',w/'candidate');subprocess.run(['git','apply',str(d/'candidate.patch')],cwd=w/'candidate',check=True)
snapshot=json.loads((d/'snapshot.json').read_text())
for variant,records in snapshot['headers'].items():assert {name:sha(w/variant/name) for name in records}==records,variant
for name,expected in snapshot['drivers'].items():
 for variant in ['base','candidate']:assert sha(w/variant/name)==expected,(variant,name)
shutil.copy2(d/'snapshot.json',w/'snapshot.json');shutil.copy2(d/'traits_codegen.cpp',w/'traits_codegen.cpp')
prepared={'baseline':baseline,'archive_sha256':hashlib.sha256(archive).hexdigest(),'snapshot_sha256':sha(w/'snapshot.json'),'snapshot_patch_sha256':sha(d/'snapshot_base.patch'),'candidate_patch_sha256':sha(d/'candidate.patch'),'all_recorded_headers_and_drivers_match':True,'prepare_only':a.prepare_only}
(w/'replay_preparation.json').write_text(json.dumps(prepared,indent=2)+'\n');print('Fixed integrated snapshot, one-line candidate and all source hashes verified',flush=True)
if a.prepare_only:raise SystemExit
folders={'conv':'convolution/convolution_mod','exp':'polynomial/exp_of_formal_power_series','bostan':'other/kth_term_of_linearly_recurrent_sequence'}
for row in json.loads((d/'confirm_raw.json').read_text())['workloads']:
 folder=a.official_root/folders[row['label']]
 assert sha(folder/'in'/(row['case']+'.in'))==row['input_sha256']
 assert sha(folder/'out'/(row['case']+'.out'))==row['answer_sha256']
 assert sha(folder/'checker')==row['checker_sha256']
old=json.loads((d/'build.json').read_text());oldroot='/workspace/cp-fps-scratch-round3/mint-inline';build={'flags':old['flags'],'compiler':subprocess.check_output([a.compiler,'--version'],text=True),'snapshot_sha256':sha(w/'snapshot.json'),'builds':{}}
commands={}
for name,record in old['builds'].items():
 if name in ['traits_gcc13','traits_clang19'] or name.startswith('asm_'):continue
 command=[value.replace(oldroot,str(w)) for value in record['command']];command[0]=a.compiler;commands[name]=command

def compile_one(item):
 name,command=item;r=subprocess.run(command,capture_output=True,text=True)
 return name,{'command':command,'returncode':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'source_sha256':sha(Path(command[-3])),'binary_sha256':sha(Path(command[-1])) if r.returncode==0 else None}
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 with concurrent.futures.ThreadPoolExecutor(2) as pool:
  for name,record in pool.map(compile_one,commands.items()):build['builds'][name]=record;assert record['returncode']==0,(name,record['stderr'])
 for name in ['traits_base','traits_candidate']:subprocess.run([str(w/name)],check=True)
(w/'build.json').write_text(json.dumps(build,indent=2)+'\n')
for filename in ['measure_first.py','confirm.py']:
 source=(d/filename).read_text().replace(oldroot,str(w)).replace('/tmp/library-checker-problems',str(a.official_root));(w/filename).write_text(source);subprocess.run(['python3',str(w/filename)],check=True)
