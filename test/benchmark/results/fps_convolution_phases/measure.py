from pathlib import Path
import fcntl,hashlib,json,os,random,statistics,subprocess,time
w=Path(__file__).resolve().parent;folder=Path(os.environ.get('FPS_OFFICIAL_ROOT','/tmp/library-checker-problems'))/'convolution/convolution_mod';variants=['current','reference','profile_current','profile_reference'];trials=21
inp=folder/'in/max_random_00.in';answer=folder/'out/max_random_00.out'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
recorded=json.loads((w/'recorded.json').read_text())
assert sha(inp)==recorded['input_sha256'] and sha(answer)==recorded['answer_sha256']
result={'metadata':json.loads((w/'metadata.json').read_text()),'build':json.loads((w/'build.json').read_text()),'analysis_only':True,'note':'Instrumented reference is modified for analysis. Original binaries/reference source are unchanged. Public profile driver expands the selected Product path and IO body using unchanged headers. All helper attributes match original O3/AVX2. Not an official ranking.', 'cpu':2,'trials':trials,'shuffle_seed':20261021,'input_sha256':sha(inp),'answer_sha256':sha(answer),'samples':{v:[] for v in variants},'clock_calibration':[]}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(20261021)
 for rep in range(trials+1):
  order=variants.copy();rng.shuffle(order)
  for variant in order:
   output=w/(variant+'.out');error=w/(variant+'.stderr')
   with inp.open('rb') as stdin,output.open('wb') as stdout,error.open('wb') as stderr:
    begin=time.perf_counter_ns();subprocess.run([str(w/variant)],stdin=stdin,stdout=stdout,stderr=stderr,check=True);elapsed=(time.perf_counter_ns()-begin)/1e6
   subprocess.run([str(folder/'checker'),str(inp),str(output),str(answer)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
   row={'whole_ms':elapsed,'checker_passed':True}
   if variant.startswith('profile_'):
    p=json.loads(error.read_text());assert sum(p['phases_ns'].values())==p['internal_ns'];row.update(p);row['external_minus_internal_ms']=elapsed-p['internal_ns']/1e6
   else:assert not error.read_bytes()
   if rep:result['samples'][variant].append(row)
 for rep in range(10):
  c=subprocess.run([str(w/'clock_calibration')],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True)
  p=json.loads(c.stderr)
  assert p['clock_calls']==1000001
  if rep:result['clock_calibration'].append(p['internal_ns']/1000000)
result['clock_step_median_ns']=statistics.median(result['clock_calibration'])
result['summary']={}
for variant,rows in result['samples'].items():
 summary={'whole_median_ms':statistics.median(r['whole_ms'] for r in rows),'output_sha256':sha(w/(variant+'.out'))}
 if variant.startswith('profile_'):
  summary.update(internal_median_ms=statistics.median(r['internal_ns']/1e6 for r in rows), external_minus_internal_median_ms=statistics.median(r['external_minus_internal_ms'] for r in rows),clock_calls=sorted({r['clock_calls'] for r in rows}),phases_median_ms={p:statistics.median(r['phases_ns'][p]/1e6 for r in rows) for p in rows[0]['phases_ns']})
  summary['sum_of_phase_medians_ms']=sum(summary['phases_median_ms'].values())
  summary['estimated_clock_overhead_ms']=(summary['clock_calls'][0]-1)*result['clock_step_median_ns']/1e6
 result['summary'][variant]=summary
result['instrumentation_differences_ms']={v:result['summary']['profile_'+v]['whole_median_ms']-result['summary'][v]['whole_median_ms'] for v in ('current','reference')}
assert len({row['output_sha256'] for row in result['summary'].values()})==1
result['headers_unchanged']=all(sha(w/'source'/name)==value for name,value in result['metadata']['snapshot_sha256'].items())
result['group_definitions']=recorded['group_definitions']
result['grouped_phase_medians_ms']={variant:{group:statistics.median(sum(row['phases_ns'][p] for p in parts)/1e6 for row in result['samples'][variant]) for group,parts in result['group_definitions'].items()} for variant in ('profile_current','profile_reference')}
(w/'results.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result['summary'],indent=2),flush=True)
print('Clock step median ns:',result['clock_step_median_ns'],'instrumentation deltas ms:',result['instrumentation_differences_ms'],'headers unchanged:',result['headers_unchanged'],flush=True)
