from pathlib import Path
import argparse,hashlib,io,json,shutil,subprocess,tarfile
p=argparse.ArgumentParser();p.add_argument('--repository',type=Path,default=Path(__file__).resolve().parents[4]);p.add_argument('--work',type=Path,required=True);p.add_argument('--official-root',type=Path,default=Path('/tmp/library-checker-problems'));p.add_argument('--references',type=Path,default=Path('/tmp/fps-reference'));p.add_argument('--prepare-only',action='store_true');a=p.parse_args();archive=Path(__file__).resolve().parent;raw=json.loads((archive/'batch_raw.json').read_text());repo=a.repository.resolve();work=a.work.resolve()
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(archive/'batch_harness.py')==raw['harness_sha256']
assert sha(archive/'driver.cpp')==raw['driver_sha256']
if work.exists() and any(work.iterdir()):raise SystemExit('work directory must be empty')
work.mkdir(parents=True,exist_ok=True);base=work/'baseline';candidate=work/'candidate';payload=subprocess.check_output(['git','archive',raw['baseline']],cwd=repo)
with tarfile.open(fileobj=io.BytesIO(payload)) as tar:tar.extractall(base,filter='data')
def verify(tree,variant):
 for name,expected in raw['sources'][variant].items():assert sha(tree/name)==expected,(variant,name)
 return len(raw['sources'][variant])
counts={'baseline':verify(base,'baseline')};shutil.copytree(base,candidate);subprocess.run(['git','apply','--no-index',str(archive/'batch_candidate.patch')],cwd=candidate,check=True);counts['candidate']=verify(candidate,'candidate')
proof={'baseline':raw['baseline'],'repository':str(repo),'work':str(work),'header_counts':counts,'original_harness_sha256':sha(archive/'batch_harness.py'),'driver_sha256':sha(archive/'driver.cpp'),'restoration':'passed'};(work/'restoration.json').write_text(json.dumps(proof,indent=2)+'\n');print(json.dumps(proof,indent=2),flush=True)
if a.prepare_only:raise SystemExit(0)
source=(archive/'batch_harness.py').read_text()
replacements={"R=Path('/workspace/cp-library')":'R=Path('+repr(str(repo))+')',"W=Path('/tmp/fps-division-round2')":'W=Path('+repr(str(work))+')',"B=Path('/tmp/cp-fps-round2-base')":'B=Path('+repr(str(base))+')',"O=Path('/tmp/library-checker-problems/polynomial')":'O=Path('+repr(str(a.official_root.resolve()/'polynomial'))+')',"Path('/tmp/fps-reference')":'Path('+repr(str(a.references.resolve()))+')',"R/'test/benchmark/results/fps_division_measurement_driver.cpp'":'Path('+repr(str(archive/'driver.cpp'))+')'}
for old,new in replacements.items():assert old in source,old;source=source.replace(old,new)
script=work/'replay_harness.py';script.write_text(source);subprocess.run(['python3',str(script)],check=True,cwd=repo)
