from pathlib import Path
import datetime,fcntl,hashlib,json,subprocess
r=Path('/workspace/cp-library');o=r/'test/benchmark/results/fps_compact';p=r/'test/benchmark/results/kth_term_submission.json';src=r/'submissions/kth_term_of_linearly_recurrent_sequence.cpp';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();old=json.loads(p.read_text());before=sha(src)
(o/'previous_submission.json').write_bytes(p.read_bytes())
assert sha(Path('/usr/bin/g++').resolve())==old['compiler_sha256']
fixtures=Path('/tmp/library-checker-problems/other/kth_term_of_linearly_recurrent_sequence');assert sha(fixtures/'checker')==old['checker_sha256']
for row in old['cases']:
 assert sha(fixtures/'in'/(row['case']+'.in'))==row['input_sha256']
 assert sha(fixtures/'out'/(row['case']+'.out'))==row['answer_sha256']
command=['/usr/bin/g++',*old['flags'],str(src),'-o','/workspace/cp-fps-256/submission-main']
with open('/tmp/cp-fps-tuning.lock','a') as lock:
 fcntl.flock(lock,fcntl.LOCK_SH)
 q=subprocess.run(command,capture_output=True,text=True);(o/'submission_compile.log').write_text(q.stdout+q.stderr);assert q.returncode==0,q.stderr
assert sha(Path('/workspace/cp-fps-256/submission-main'))==old['binary_sha256']
assert sha(src)==before
record={k:old[k] for k in ['source','generated_from','generation','compiler','compiler_sha256','flags','binary_sha256','cases','checker','checker_sha256','result','cases_passed','judge_submission_performed','judge_fastest_confirmed']}
record.update(recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256=before,source_bytes=src.stat().st_size,library_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=r,text=True).strip(),compression=['python3','tools/compact_fps.py','--limit','262144'],compression_tool_sha256=sha(r/'tools/compact_fps.py'),build_command=command,source_unchanged=True,official_case_evidence_reused={'record':'fps_compact/previous_submission.json','record_sha256':sha(o/'previous_submission.json'),'whole_executable_identical':True,'compiler_checker_inputs_answers_hashes_match':True})
p.write_text(json.dumps(record,indent=2)+'\n');print('PASS compact submission matches executable verified on 20 official cases; bytes',src.stat().st_size,flush=True)
