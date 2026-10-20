from pathlib import Path
import hashlib,importlib.util,itertools,json,subprocess,fcntl
R=Path('/workspace/cp-library');O=R/'test/benchmark/results/fps_compact';W=Path('/workspace/cp-fps-256/integration');W.mkdir();sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();old=json.loads((O/'result.json').read_text());assert old['status']=='passed'
spec=importlib.util.spec_from_file_location('pack',R/'tools/compact_fps.py');pack=importlib.util.module_from_spec(spec);spec.loader.exec_module(pack)
assert sha(R/'tools/compact_fps.py')==old['source_hashes']['tools/compact_fps.py'];assert sha(R/'test/benchmark/fps_compact_check.py')==old['source_hashes']['test/benchmark/fps_compact_check.py'];assert sha(Path('/usr/bin/g++').resolve())==old['compilers']['/usr/bin/g++']['sha256']
audit={'revision':subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip(),'unchanged_bundles':[],'changed_bundles':[],'status':'running'}
for row in old['sizes']:
 name=row['source'];q=subprocess.run(['/tmp/fps-venv/bin/oj-bundle','-I','.',name],cwd=R,capture_output=True,check=True);h=hashlib.sha256(q.stdout).hexdigest()
 if h==row['before']['sha256']:audit['unchanged_bundles'].append(name);continue
 text=q.stdout.decode();packed=pack.compact(text);new={'source':name,'before':{'bytes':len(q.stdout),'sha256':h},'after':{'bytes':len(packed.encode()),'sha256':hashlib.sha256(packed.encode()).hexdigest()},'variants':{}};pp=[]
 for variant,data in [('before',text),('after',packed)]:
  p=W/variant/Path(name).stem;p.mkdir(parents=True);src=p/'source.cpp';src.write_text(data);cmd=['/usr/bin/g++',*old['flags'],'-ffile-prefix-map='+str(p)+'=/source',str(src)]
  with open('/tmp/cp-fps-tuning.lock','a') as lock,(p/'compile.log').open('w') as log:
   fcntl.flock(lock,fcntl.LOCK_SH);subprocess.run([*cmd,'-o',str(p/'main')],stdout=log,stderr=subprocess.STDOUT,check=True);z=subprocess.check_output([*cmd,'-E','-P'],text=True,stderr=log)
  pp.append(z);new['variants'][variant]={'command':cmd,'binary_sha256':sha(p/'main'),'preprocessed_sha256':hashlib.sha256(z.encode()).hexdigest(),'log_sha256':sha(p/'compile.log')}
 assert new['variants']['before']['binary_sha256']==new['variants']['after']['binary_sha256']
 for a,b in itertools.zip_longest(*( (m[0] for m in pack.TOKEN.finditer(text)) for text in pp)):assert a==b,(a,b)
 new['preprocessed_tokens_identical']=True;audit['changed_bundles'].append(new)
assert len(audit['changed_bundles'])==1 and audit['changed_bundles'][0]['source']=='test/multiplicative_convolution.test.cpp'
sentinel=W/'limit-output.cpp';sentinel.write_text('keep existing file\n');before=sha(sentinel);z=subprocess.run(['python3',str(R/'tools/compact_fps.py'),'/workspace/cp-fps-256/verification/before/fps_linear_recurrence.test/source.cpp','--limit','1','-o',str(sentinel)],capture_output=True,text=True);assert z.returncode!=0 and sha(sentinel)==before and 'is not below 1 bytes' in z.stderr;audit['limit_rejects_before_overwriting_output']=True
src=R/'submissions/kth_term_of_linearly_recurrent_sequence.cpp';expected=next(x for x in old['sizes'] if x['source']=='test/fps_linear_recurrence.test.cpp')['after'];assert sha(src)==expected['sha256'] and src.stat().st_size==expected['bytes'];audit['submission_matches_verified_output']=True
previous=json.loads((O/'previous_submission.json').read_text());raw=subprocess.check_output(['git','show',old['revision']+':submissions/kth_term_of_linearly_recurrent_sequence.cpp'],cwd=R);assert hashlib.sha256(raw).hexdigest()==previous['source_sha256'];audit['previous_official_case_source_matches_base_revision']=True
assert sha(R/'tools/compact_fps.py')==old['source_hashes']['tools/compact_fps.py'];audit['status']='passed';(O/'integration.json').write_text(json.dumps(audit,indent=2)+'\n');print('PASS integration;',len(audit['unchanged_bundles']),'identical bundles;',len(audit['changed_bundles']),'updated include bundle checked; CLI size rejection and artifact hashes passed',flush=True)
