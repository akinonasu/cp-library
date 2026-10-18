from pathlib import Path
import argparse,hashlib,io,json,random,shutil,subprocess,tarfile
p=argparse.ArgumentParser();p.add_argument('--repository',type=Path,default=Path(__file__).resolve().parents[4]);p.add_argument('--work',type=Path,required=True);p.add_argument('--official-root',type=Path,default=Path('/tmp/library-checker-problems'));p.add_argument('--references',type=Path,default=Path('/tmp/fps-reference'));p.add_argument('--compiler',default='g++');p.add_argument('--prepare-only',action='store_true');a=p.parse_args();archive=Path(__file__).resolve().parent;raw=json.loads((archive/'final_raw.json').read_text());repo=a.repository.resolve();work=a.work.resolve();sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(archive/'final_harness.py')==raw['harness_sha256'];assert sha(archive/'driver.cpp')==raw['driver_sha256']
if work.exists() and any(work.iterdir()):raise SystemExit('work directory must be empty')
work.mkdir(parents=True,exist_ok=True);base=work/'baseline';candidate=work/'candidate';payload=subprocess.check_output(['git','archive',raw['baseline']],cwd=repo)
with tarfile.open(fileobj=io.BytesIO(payload)) as tar:tar.extractall(base,filter='data')
def verify(tree,variant):
 for name,expected in raw['headers_before'][variant].items():assert sha(tree/name)==expected,(variant,name)
 return len(raw['headers_before'][variant])
counts={'baseline':verify(base,'baseline')};shutil.copytree(base,candidate);subprocess.run(['git','apply','--no-index',str(archive/'final_candidate.patch')],cwd=candidate,check=True);counts['candidate']=verify(candidate,'candidate')
for name in ['driver.cpp','standalone.cpp','unit_gcd.cpp']:shutil.copyfile(archive/name,work/name)
n=500000;rng=random.Random(20261019);series=str(n)+' '+str(n)+'\n'+' '.join(str(rng.randrange(998244353)) for _ in range(n))+'\n1 '+' '.join(str(rng.randrange(998244353)) for _ in range(n-1))+'\n';(work/'series.in').write_text(series);series_sha=next(row['input_sha256'] for row in raw['workloads'] if row['label']=='series');assert sha(work/'series.in')==series_sha
proof={'baseline':raw['baseline'],'repository':str(repo),'work':str(work),'header_counts':counts,'original_harness_sha256':sha(archive/'final_harness.py'),'driver_sha256':sha(archive/'driver.cpp'),'series_input_sha256':series_sha,'restoration':'passed'};(work/'restoration.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof,indent=2),flush=True)
if a.prepare_only:raise SystemExit(0)
for row in raw['workloads']:
 if row['label']=='series':continue
 problem='division_of_polynomials' if row['label']=='division' else 'inv_of_polynomials'
 folder=a.official_root.resolve()/'polynomial'/problem
 assert sha(folder/'in'/(row['case']+'.in'))==row['input_sha256']
 if row['answer_sha256']:assert sha(folder/'out'/(row['case']+'.out'))==row['answer_sha256']
old_work='/tmp/fps-division-round2';old_base='/tmp/cp-fps-round2-base';old_candidate='/tmp/fps-division-round2/dividend_scoped_candidate';old_reference='/tmp/fps-reference'
def remap(value):
 for old,new in [(old_candidate,str(candidate)),(old_base,str(base)),(old_work,str(work)),(old_reference,str(a.references.resolve()))]:value=value.replace(old,new)
 return value
builds={}
def compile_record(name,record):
 cmd=record['command'][:]
 if cmd[0]=='flock':cmd=cmd[3:]
 cmd=[a.compiler,*map(remap,cmd[1:])]
 if record.get('source_sha256'):assert sha(Path(cmd[-3]))==record['source_sha256']
 subprocess.run(['flock','-s','/tmp/cp-fps-tuning.lock',*cmd],cwd=repo,check=True);binary=Path(cmd[cmd.index('-o')+1]);builds[name]={'command':cmd,'binary_sha256':sha(binary),'source_sha256':sha(work/'driver.cpp') if 'reference' not in name else sha(Path(cmd[-3]))}
for name in ['division_baseline','inverse_mod_baseline','gcd_baseline','division_reference','inverse_mod_reference']:compile_record(name,raw['baseline_builds'][name])
(work/'results.json').write_text(json.dumps({'builds':builds},indent=2)+'\n');compile_record('series_round2_baseline',raw['series_baseline_build']);(work/'series_build.json').write_text(json.dumps({'series_round2_baseline':builds['series_round2_baseline']},indent=2)+'\n')
def adapt(source):
 replacements={"R=Path('/workspace/cp-library')":'R=Path('+repr(str(repo))+')',"W=Path('/tmp/fps-division-round2')":'W=Path('+repr(str(work))+')',"B=Path('/tmp/cp-fps-round2-base')":'B=Path('+repr(str(base))+')',"C=W/'dividend_scoped_candidate'":"C=W/'candidate'","O=Path('/tmp/library-checker-problems/polynomial')":'O=Path('+repr(str(a.official_root.resolve()/'polynomial'))+')',"Path('/tmp/fps-division/series.in')":"(W/'series.in')","R/'test/unit/fps_division_blocked.cpp'":'Path('+repr(str(archive/'final_unit_division.cpp'))+')',"'g++'":repr(a.compiler)}
 for old,new in replacements.items():source=source.replace(old,new)
 return source
check=work/'replay_final_check.py';check.write_text(adapt((archive/'final_check_harness.py').read_text()));subprocess.run(['python3',str(check)],cwd=repo,check=True)
bench=work/'replay_final_harness.py';bench.write_text(adapt((archive/'final_harness.py').read_text()));subprocess.run(['python3',str(bench)],cwd=repo,check=True)
