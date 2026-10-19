from pathlib import Path
import difflib,gzip,hashlib,json,shutil,subprocess
W=Path('/workspace/cp-fps-round4/borrow');O=W/'artifacts';O.mkdir(exist_ok=True);(O/'sources').mkdir(exist_ok=True)
H='components/fps_convolution.hpp';baseline='7521ea8703a362aaf3ccd370188fccab86050848';variants=['range_import','resize_control','range_transform','reserve_only','huge_only','range_transform_inline','batch_construct','tail_store','range_lazy','capacity_1m','capacity_2m','range_inplace']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
B=W/'before';meta={'baseline':baseline,'production_edited_by_this_work':False,'candidate_status':'capacity_1m passed 21+41 trials and validation; adoption belongs to root. Other candidates rejected.','baseline_headers':{str(p.relative_to(B)):sha(p) for f in ['fps','components','math','convolution','test'] for p in sorted((B/f).rglob('*.hpp'))},'candidates':{},'records':{},'compiler':subprocess.check_output(['g++','--version'],text=True)}
for name in ['before',*variants]:
 src=W/name/H
 with (O/'sources'/(name+'.hpp.gz')).open('wb') as raw:
  with gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as z:z.write(src.read_bytes())
 if name=='before':continue
 patch=O/(name+'.patch');patch.write_text(''.join(difflib.unified_diff((B/H).read_text().splitlines(True),src.read_text().splitlines(True),fromfile='a/'+H,tofile='b/'+H)))
 meta['candidates'][name]={'header':H,'header_sha256':sha(src),'patch_sha256':sha(patch),'actual_source':'sources/'+name+'.hpp.gz','status':'not adopted'}
for src,dst in [('explore9/result.json','range_resize_9.json'),('more9/result.json','transform_advice_9.json'),('inline9/result.json','transform_inline_9.json'),('batch_tail9/result9.json','batch_tail_9.json'),('batch_tail9/result.json','batch_tail_21.json'),('lazy9/result.json','lazy_9.json'),('capacity9/result9.json','capacity_9.json'),('capacity9/result21.json','capacity_21.json'),('capacity9/result.json','capacity_41.json'),('inplace9/result.json','range_inplace_9.json')]:
 shutil.copyfile(W/src,O/dst);meta['records'][dst]={'original':str(W/src),'sha256':sha(O/dst)}
for name in ['explore9.log','more9.log','inline9.log','batch_tail9.log','batch_tail21.log','lazy9.log','capacity9.log','setup.py','setup_more.py','setup_batch_tail.py','setup_lazy.py','setup_capacity.py','setup_inplace.py','capacity21.log','capacity41.log','inplace9.log','resources.py','resources.json','resources.log','validate_capacity.py','capacity_validation.json','capacity_validation.log','capacity_normal.log','capacity_sanitize.log','whole.py','whole_v1.py']:
 shutil.copyfile(W/name,O/name)
shutil.copyfile(W/'tail_store/test/unit/fps_convolution_tail_store.cpp',O/'tail_store_forced_offset.cpp')
meta['tail_store_unit']='Prepared source only; not built or run because whole-process improvement was not stable.'
ref=Path('/tmp/fps-reference/convolution_mod/409465.cpp')
with (O/'sources/reference409465.cpp.gz').open('wb') as raw:
 with gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as z:z.write(ref.read_bytes())
meta['reference']={'id':409465,'sha256':sha(ref),'source':'sources/reference409465.cpp.gz'}
for p in ['test/convolution.test.cpp','test/fps_io.hpp']:
 shutil.copyfile(B/p,O/'sources'/Path(p).name)
shutil.copyfile(Path(__file__),O/'archive_initial.py')
meta['candidates']['capacity_1m']['status']='eligible for root review; shared production not edited'
# Retain disassembly evidence for the failed input-iterator construction path.
for name,folder in [('range_transform','more9'),('range_transform_inline','inline9')]:
 binary=W/folder/name;text=subprocess.check_output(['objdump','-dC','--no-show-raw-insn',str(binary)],text=True)
 selected=[]
 for block in text.split('\n\n'):
  lines=block.splitlines();first=lines[0] if lines else ''
  if 'multiply_detail::run_borrowed(' in first:selected.append(block)
 (O/(name+'_codegen.txt')).write_text('\n\n'.join(selected)+'\n')
meta['files']={str(p.relative_to(O)):sha(p) for p in sorted(O.rglob('*')) if p.is_file() and p.name!='metadata.json'}
(O/'metadata.json').write_text(json.dumps(meta,indent=2)+'\n');print('archived',len(meta['files']),'files')
