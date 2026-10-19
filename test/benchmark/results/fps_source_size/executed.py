import argparse,ast,fcntl,hashlib,io,json,os,re,shlex,shutil,subprocess,tarfile,time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
R=Path('/workspace/cp-library');W=Path('/workspace/cp-fps-size/verification-final');W.mkdir(exist_ok=True);OUT=R/'test/benchmark/results/fps_source_size';OUT.mkdir(parents=True,exist_ok=True)
BASE='3d7dfe2caa96a00c8e78246b26a29bcafa41926c';BUNDLE=Path('/tmp/fps-venv/bin/oj-bundle');FLAGS=['-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell'];sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();lit=r'"(?:[^"\\]|\\.)*"';asm=re.compile(r'\basm\s*(?:volatile\s*)?\(\s*(?P<body>(?:'+lit+r'\s*)+)')
changed=['components/fps_blocked.hpp','components/fps_composition.hpp','components/fps_convolution.hpp','components/fps_sqrt_blocked.hpp'];drivers=json.loads(Path('/workspace/cp-fps-size/affected-drivers.json').read_text());roots={v:W/v for v in ['before','after']}
raw=subprocess.check_output(['git','archive',BASE,'components','fps','convolution','math','data_structure','graph','string','test/fps_io.hpp',*drivers],cwd=R)
for v,p in roots.items():
 assert not p.exists()
 with tarfile.open(fileobj=io.BytesIO(raw)) as a:a.extractall(p,filter='data')
 if v=='after':
  for name in [*changed,'components/fps_asm.hpp']:
   shutil.copy2(R/name,p/name)
 for folder in ['bundles','binaries','logs']: (p/folder).mkdir()
result={'baseline':BASE,'harness_sha256':sha(Path(__file__)),'flags':FLAGS,'source_hashes':{v:{str(p.relative_to(root)):sha(p) for p in sorted(root.rglob('*.hpp'))} for v,root in roots.items()},'bundle_tool':{'path':str(BUNDLE),'sha256':sha(BUNDLE),'version':subprocess.check_output(['/tmp/fps-venv/bin/python','-c','import importlib.metadata;print(importlib.metadata.version("online-judge-verify-helper"))'],text=True).strip()},'drivers':drivers,'sizes':[],'checks':[],'compilers':{},'status':'running','note':'Compile both variants with identical file basenames and mapped task-directory prefixes. No optimization, dispatch, asm qualifier, template bytes, constraints or clobbers may change.'}
def save(): (OUT/'result.json').write_text(json.dumps(result,indent=2)+'\n')
for c in ['/usr/bin/g++','/tmp/fps-gcc13/toolchain/usr/bin/g++-13','/tmp/fps-clang/toolchain/usr/bin/clang++-19']:
 p=Path(c);result['compilers'][c]={'sha256':sha(p.resolve()),'path':str(p.resolve()),'version':subprocess.check_output([c,'--version'],text=True)}
for name in drivers:
 row={'source':name}
 for v,root in roots.items():
  target=root/'bundles'/Path(name).name
  with target.open('wb') as so,(root/'logs'/(Path(name).stem+'.bundle.log')).open('wb') as se:
   subprocess.run([str(BUNDLE),'-I','.',name],cwd=root,stdout=so,stderr=se,check=True)
  data=target.read_bytes();assert not re.search(rb'^\s*#\s*include\s*"',data,re.M)
  row[v]={'bytes':len(data),'sha256':sha(target)}
 assert row['after']['bytes']<512*1024,row
 result['sizes'].append(row);print('size',name,row['before']['bytes'],row['after']['bytes'],flush=True)
save()
for header in sorted((R/'fps').glob('*.hpp')):
 row={'source':str(header.relative_to(R)),'kind':'header-with-fast-io'}
 for v,root in roots.items():
  driver=root/'header.cpp';driver.write_text('#include "'+row['source']+'"\n#include "test/fps_io.hpp"\nint main() {}\n')
  q=subprocess.run([str(BUNDLE),'-I','.','header.cpp'],cwd=root,capture_output=True,check=True);row[v]={'bytes':len(q.stdout),'sha256':hashlib.sha256(q.stdout).hexdigest()}
 assert row['after']['bytes']<512*1024,row
 result['sizes'].append(row)
save()
def check(job):
 compiler,name,extra=job;label=Path(compiler).name.replace('+','p')+('_portable' if extra else '')+'_'+Path(name).stem;row={'compiler':compiler,'source':name,'extra_flags':extra,'variants':{}}
 for v,root in roots.items():
  src=root/'bundles'/Path(name).name;binary=root/'binaries'/label;log=root/'logs'/(label+'.log');cmd=[compiler,*FLAGS,*extra,'-ffile-prefix-map='+str(root)+'=/library',str(src),'-o',str(binary)]
  start=time.monotonic()
  with open('/tmp/cp-fps-tuning.lock','a') as lock,log.open('w') as out:
   fcntl.flock(lock,fcntl.LOCK_SH);p=subprocess.run(cmd,cwd=root,stdout=out,stderr=subprocess.STDOUT)
  assert p.returncode==0,(cmd,log.read_text()[-3000:]);row['variants'][v]={'command':cmd,'binary_sha256':sha(binary),'bytes':binary.stat().st_size,'log_sha256':sha(log),'seconds':time.monotonic()-start}
 assert row['variants']['before']['binary_sha256']==row['variants']['after']['binary_sha256'],row
 return row
jobs=[('/usr/bin/g++',n,[]) for n in drivers]
jobs +=[('/tmp/fps-gcc13/toolchain/usr/bin/g++-13',n,[]) for n in ['test/convolution.test.cpp','test/fps_compose.test.cpp','test/fps_interpolate.test.cpp','test/fps_sqrt.test.cpp']]
jobs +=[('/tmp/fps-clang/toolchain/usr/bin/clang++-19',n,[]) for n in ['test/convolution.test.cpp','test/fps_compose.test.cpp']]
jobs +=[('/usr/bin/g++',n,['-U__linux__']) for n in ['test/convolution.test.cpp','test/fps_compose.test.cpp']]
with ThreadPoolExecutor(max_workers=3) as pool:
 for row in pool.map(check,jobs):
  result['checks'].append(row);save();print('identical',row['compiler'],row['source'],row['extra_flags'],flush=True)
for v,root in roots.items():
 source=root/'bundles/fps_compose.test.cpp';cmd=['/usr/bin/g++',*FLAGS,'-ffile-prefix-map='+str(root)+'=/library','-E','-P',str(source)]
 with open('/tmp/cp-fps-tuning.lock','a') as lock:
  fcntl.flock(lock,fcntl.LOCK_SH);text=subprocess.check_output(cmd,cwd=root,text=True)
 templates=[{'volatile':bool(re.match(r'asm\s+volatile',m[0])),'bytes':''.join(ast.literal_eval(s) for s in re.findall(lit,m['body']))} for m in asm.finditer(text)]
 if v=='before':previous=templates
 else:assert previous==templates
 result.setdefault('assembly_templates',{})[v]=[{'volatile':x['volatile'],'bytes':len(x['bytes']),'sha256':hashlib.sha256(x['bytes'].encode()).hexdigest()} for x in templates]
final_sources={str(p.relative_to(R)):sha(p) for p in [R/x for x in [*changed,'components/fps_asm.hpp']]};assert all(result['source_hashes']['after'][p]==h for p,h in final_sources.items())
result['live_source_hashes_match']=True;result['compiler_hashes_match']=all(sha(Path(c).resolve())==row['sha256'] for c,row in result['compilers'].items());assert result['compiler_hashes_match'];result['status']='passed';save();print('PASS',len(result['checks']),'whole executables identical;',len(result['sizes']),'bundles below 512 KiB;',len(previous),'asm templates identical',flush=True)
