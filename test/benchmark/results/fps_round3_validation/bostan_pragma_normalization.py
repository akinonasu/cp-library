from pathlib import Path
from datetime import datetime,timezone
import argparse,fcntl,hashlib,json,shutil,subprocess,time

p=argparse.ArgumentParser()
p.add_argument('--parent',type=Path,default=Path(__file__).resolve().parents[4]/'test/benchmark/results/fps_round3_all_speed.json')
p.add_argument('--work',type=Path,default=Path('/workspace/cp-fps-scratch-round3/fps-round3-all-speed'))
p.add_argument('--output',type=Path,default=Path(__file__).with_suffix('.json'))
a=p.parse_args();a.parent=a.parent.resolve();a.work=a.work.resolve();a.output=a.output.resolve();root=Path(__file__).resolve().parents[4];sha=lambda f:hashlib.sha256(f.read_bytes()).hexdigest();last_status=0
print('Waiting for all 16 formal benchmark workloads; no timings rerun',flush=True)
while True:
 try:
  parent=json.loads(a.parent.read_text());expected=sum(name.endswith('_reference') for name in parent.get('sources',{}));finished=len(parent.get('judge',[]))
 except (FileNotFoundError,json.JSONDecodeError):expected,finished=16,0
 if expected==16 and finished==expected:break
 if time.monotonic()-last_status>=15:print('Formal benchmark completed',finished,'/',expected,flush=True);last_status=time.monotonic()
 time.sleep(2)
print('All 16 workload records present; waiting for shared rebuild lock',flush=True)
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 parent=json.loads(a.parent.read_text());assert len(parent['judge'])==16;parent_sha=sha(a.parent)
 record=parent['sources']['kth_term_of_linearly_recurrent_sequence_current'];command=record['command'];binary=Path(command[-1]);source=Path(command[-3]);tree=a.work/'current';rel='components/fps_bostan.hpp';header=tree/rel;canonical=root/rel
 assert binary==a.work/'kth_term_of_linearly_recurrent_sequence_current'
 assert sha(binary)==record['binary_sha256'] and sha(source)==record['source_sha256']
 before_headers={name:sha(tree/name) for name in parent['sources']['current']};assert before_headers==parent['sources']['current']
 old=header.read_bytes();new=canonical.read_bytes()
 assert old.replace(b'#pragma GCC target("avx2")',b'_Pragma("GCC target(\\"avx2\\")")')==new
 assert sha(header)=='bbe4528ee226bcf0ea90d143f0b5832ef6f69b2162b448ec845d249349bc7be3'
 assert sha(canonical)=='bda7e2f7b2189023cc5da6b695eddc5bcde4ff81eb8407d933c0cf18fb13e258'
 assert subprocess.check_output([command[0],'--version'],text=True).splitlines()[0]==parent['compiler']
 before_source=a.output.with_name('bostan_pragma_before.source');after_source=a.output.with_name('bostan_pragma_canonical.source');before_source.write_bytes(old);after_source.write_bytes(new)
 saved_binary=a.work/'kth_term_of_linearly_recurrent_sequence_current.before_pragma';shutil.copy2(binary,saved_binary)
 j={'started_at':datetime.now(timezone.utc).isoformat(),'purpose':'After completed formal timing, normalize only the copied current Bostan pragma and prove byte-identical LR executable; historical raw result is immutable','harness_sha256':sha(Path(__file__)),'parent_raw':{'path':str(a.parent),'sha256':parent_sha,'completed_workloads':len(parent['judge']),'historical_sources_current':parent['sources']['current'],'historical_binary_record':record},'compiler':parent['compiler'],'flags':parent['flags'],'historical_header':{'path':str(header),'sha256':sha(header),'archived_source':str(before_source),'archived_sha256':sha(before_source)},'canonical_header':{'path':str(canonical),'sha256':sha(canonical),'archived_source':str(after_source),'archived_sha256':sha(after_source)},'header_change':'Only #pragma GCC target("avx2") to equivalent _Pragma spelling','binary_before':{'path':str(saved_binary),'sha256':sha(saved_binary),'bytes':saved_binary.stat().st_size},'command':command,'lock':'shared /tmp/cp-fps-tuning.lock; held only after formal timing finished'}
 header.write_bytes(new)
 run=subprocess.run(command,cwd=root,capture_output=True,text=True);j['build']={'returncode':run.returncode,'stdout':run.stdout,'stderr':run.stderr};assert run.returncode==0,run.stderr
 cmp=subprocess.run(['cmp','--silent',str(saved_binary),str(binary)]);j['cmp_returncode']=cmp.returncode;j['binary_after']={'path':str(binary),'sha256':sha(binary),'bytes':binary.stat().st_size}
 j['byte_identical']=cmp.returncode==0 and binary.read_bytes()==saved_binary.read_bytes();assert j['byte_identical']
 expected_headers=dict(before_headers);expected_headers[rel]=sha(canonical);j['normalized_sources_current']={name:sha(tree/name) for name in expected_headers};assert j['normalized_sources_current']==expected_headers
 j['parent_raw_unchanged']=sha(a.parent)==parent_sha;assert j['parent_raw_unchanged'];assert sha(canonical)==j['canonical_header']['sha256'];j['finished_at']=datetime.now(timezone.utc).isoformat();a.output.write_text(json.dumps(j,indent=2)+'\n')
 print('Rebuilt current LR executable byte-identical:',j['binary_after']['sha256'],flush=True)
 print('Historical parent SHA and all original measured values preserved:',parent_sha,flush=True)
