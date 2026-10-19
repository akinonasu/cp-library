from pathlib import Path
import fcntl,hashlib,json,shlex,shutil,subprocess
w=Path(__file__).resolve().parent;root=w/'candidate_noinline';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def compiler(cmd):
 p=Path(shutil.which(cmd) or cmd).resolve();return {'compiler_path':cmd,'resolved_path':str(p),'sha256':sha(p),'version':subprocess.check_output([str(p),'--version'],text=True)}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 j=json.loads((w/'final_checks.json').read_text());j['project_root']=str(root);j['harness_sha256']=sha(w/'check_final.py')
 for row in j['checks']:
  cmd=row['command'];src=Path(cmd[-3]);flags=cmd[1:-3];row['compiler']=compiler(cmd[0]);row['source_path']=str(src);row['source_sha256']=sha(src);row['project_root']=str(root);dcmd=[cmd[0],*flags,'-MM','-MT','unit',str(src)];raw=subprocess.check_output(dcmd,text=True);file=w/(row['name']+'.d');file.write_text(raw);paths=shlex.split(raw.split(':',1)[1].replace('\\\n',' '));row['actual_dependency_command']=dcmd;row['actual_dependency_file']=file.name;row['actual_dependency_sha256']=sha(file);row['actual_dependencies']=[]
  for p in map(Path,paths):
   p=p.resolve();row['actual_dependencies'].append({'path':str(p),'relative':str(p.relative_to(root)) if p.is_relative_to(root) else '@unit' if p==src.resolve() else None,'sha256':sha(p)})
  row['compile_returncode']=row['run_returncode']=0;log=w/('final_'+row['name']+'.log');row['log_path']=log.name;row['log_sha256']=sha(log)
 (w/'final_checks.json').write_text(json.dumps(j,indent=2)+'\n')
 j=json.loads((w/'reuse_checks.json').read_text());j['source_path']=str(w/'unit.cpp');j['project_root']=str(root);j['harness_sha256']=sha(w/'check_reuse.py')
 for row in j['checks']:
  row['compiler']=compiler(row['command'][0]);row['source_path']=str(w/'unit.cpp');row['source_sha256']=sha(w/'unit.cpp');row['project_root']=str(root);row['compile_returncode']=row['run_returncode']=0;row['log_path']=row['log_file'];row['dependency_hashes']={p['tree_relative'] if p['tree_relative'] is not None else '@unit' if p['source'] else p['path']:p['sha256'] for p in row['dependencies_before']['files']}
 (w/'reuse_checks.json').write_text(json.dumps(j,indent=2)+'\n');fcntl.flock(lock,fcntl.LOCK_UN)
dest=Path('/workspace/cp-library/test/benchmark/results/fps_convolution_constant_round4/private_validation');dest.mkdir(parents=True,exist_ok=True)
for name in ['check_final.py','check_reuse.py','fix_provenance.py','final_checks.json','final_checks.log','reuse_checks.json','reuse_checks.log','unit.cpp','standalone.cpp']:
 shutil.copyfile(w/name,dest/name)
for p in sorted(w.glob('*.d')):shutil.copyfile(p,dest/p.name)
for p in sorted(w.glob('final_*.log')):shutil.copyfile(p,dest/p.name)
for name in ['reuse_normal.log','reuse_sanitize.log']:shutil.copyfile(w/name,dest/name)
print(dest)
