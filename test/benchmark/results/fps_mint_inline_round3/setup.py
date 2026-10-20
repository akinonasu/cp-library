from pathlib import Path
from datetime import datetime,timezone
import hashlib,json,shutil,subprocess
r=Path('/workspace/cp-library');w=Path('/workspace/cp-fps-scratch-round3/mint-inline');sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
files=subprocess.check_output(['rg','--files','-g','*.hpp'],cwd=r,text=True).splitlines();before={f:sha(r/f) for f in files}
for f in files:
 dst=w/'base'/f;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(r/f,dst)
assert before=={f:sha(r/f) for f in files},'Production changed while taking snapshot'
for f in ['test/convolution.test.cpp','test/fps_exp.test.cpp','test/fps_linear_recurrence.test.cpp','test/unit/mint.cpp']:
 dst=w/'base'/f;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(r/f,dst)
shutil.copytree(w/'base',w/'candidate');p=w/'candidate/math/mint.hpp';s=p.read_text();assert s.count('\tMint() = default;')==1;s=s.replace('\tMint() = default;','\t[[gnu::always_inline]] Mint() = default;');p.write_text(s)
j={'created_at':datetime.now(timezone.utc).isoformat(),'production_git_head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=r,text=True).strip(),'production_dirty':subprocess.check_output(['git','status','--short'],cwd=r,text=True),'purpose':'Current production header snapshot plus exactly one default-constructor always_inline attribute; no production mint edits','headers':{'base':before,'candidate':{f:sha(w/'candidate'/f) for f in files}},'drivers':{f:sha(w/'base'/f) for f in ['test/convolution.test.cpp','test/fps_exp.test.cpp','test/fps_linear_recurrence.test.cpp','test/unit/mint.cpp']}}
(w/'snapshot.json').write_text(json.dumps(j,indent=2)+'\n')
