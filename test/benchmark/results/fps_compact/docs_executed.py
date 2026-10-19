from pathlib import Path
import importlib.util,json,subprocess,hashlib,os
R=Path('/workspace/cp-library');O=R/'test/benchmark/results/fps_compact';spec=importlib.util.spec_from_file_location('size_ci',R/'.verify-helper/ci.py');ci=importlib.util.module_from_spec(spec);spec.loader.exec_module(ci);plan=ci.build_plan(R,environment='fps-compact-docs');plan.update(headers=[],bundles=[],unit=[],sanitize=[],verification=[],full=False,reason='Only documentation text and submission packaging changed');selection=O/'docs_selection.json';selection.write_text(json.dumps(plan,indent=2)+'\n');python='/tmp/fps-venv/bin/python';commands=[[python,'.verify-helper/ci.py','docs','--selection',str(selection)],[python,'.verify-helper/check-docs.py','--selection',str(selection)]];result={'commands':[]};sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();files=[R/'README.md',R/'docs/fps.md',R/'submissions/README.md'];before={str(p.relative_to(R)):sha(p) for p in files};env=os.environ.copy();env['PATH']='/tmp/cp-fps-docs-bin:'+env['PATH']
for i,cmd in enumerate(commands):
 log=O/('docs'+str(i)+'.log')
 with log.open('w') as out:q=subprocess.run(cmd,cwd=R,env=env,stdout=out,stderr=subprocess.STDOUT)
 result['commands'].append({'command':cmd,'returncode':q.returncode,'log_sha256':sha(log)});assert q.returncode==0,(cmd,log)
assert all(sha(R/name)==h for name,h in before.items());result.update(source_hashes=before,status='passed');(O/'docs.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS documentation generation and metadata')
