from pathlib import Path
import argparse,fcntl,hashlib,json,os,random,statistics,subprocess,time
w=Path(__file__).resolve().parent
p=argparse.ArgumentParser();p.add_argument('--trials',type=int,default=9);p.add_argument('--cases',nargs='+',default=['max_random_00','max_random_01']);p.add_argument('--seed',type=int,default=20261040);p.add_argument('--output',default='results_9.json');a=p.parse_args()
folder=Path('/tmp/library-checker-problems/convolution/convolution_mod')
variants=['current','reference','profile_current','profile_reference']
groups={'io_setup':['io_setup_header','output_setup'],'input_alloc_parse_mint_scratch':['input_public_alloc','input_parse','input_mint_build','input_scratch_release'],'result_plan_copy_kernel_move':['result_vector','plan_alloc_init','copy_input','multiply_roots','result_move_resize'],'output_convert_format_write':['output_convert','output_format','output_write'],'release_flush':['plan_release','result_release','public_inputs_release','writer_flush_release','input_release']}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
result={'metadata':json.loads((w/'metadata.json').read_text()),'build':json.loads((w/'build.json').read_text()),'analysis_only':True,'timer':'continuous CLOCK_MONOTONIC steps; sum phase equals integer internal_ns; diagnostics outside final timestamp','cpu':2,'trials':a.trials,'seed':a.seed,'groups':groups,'cases':[],'orders':{},'clock_calibration':[]}
with open('/tmp/cp-fps-tuning.lock','w') as lock:
 fcntl.flock(lock,fcntl.LOCK_EX);os.sched_setaffinity(0,{2});rng=random.Random(a.seed)
 for case in a.cases:
  inp=folder/('in/'+case+'.in');answer=folder/('out/'+case+'.out')
  record={'name':case,'input_sha256':sha(inp),'answer_sha256':sha(answer),'samples':{v:[] for v in variants},'orders':[]}
  for rep in range(a.trials+1):
   order=variants.copy();rng.shuffle(order);record['orders'].append(order)
   for variant in order:
    output=w/(variant+'.out');error=w/(variant+'.stderr')
    with inp.open('rb') as stdin,output.open('wb') as stdout,error.open('wb') as stderr:
     begin=time.perf_counter_ns();subprocess.run([str(w/variant)],stdin=stdin,stdout=stdout,stderr=stderr,check=True);elapsed=(time.perf_counter_ns()-begin)/1e6
    subprocess.run([str(folder/'checker'),str(inp),str(output),str(answer)],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    row={'whole_ms':elapsed,'checker_passed':True}
    if variant.startswith('profile_'):
     prof=json.loads(error.read_text());assert sum(prof['phases_ns'].values())==prof['internal_ns'];assert sorted(sum(groups.values(),[]))==sorted(prof['phases_ns']);row.update(prof);row['external_minus_internal_ms']=elapsed-prof['internal_ns']/1e6
    else:assert not error.read_bytes()
    if rep:record['samples'][variant].append(row)
  record['summary']={}
  for variant,rows in record['samples'].items():
   summary={'whole_ms':statistics.median(r['whole_ms'] for r in rows),'output_sha256':sha(w/(variant+'.out'))}
   if variant.startswith('profile_'):
    summary.update(internal_ms=statistics.median(r['internal_ns']/1e6 for r in rows),residual_ms=statistics.median(r['external_minus_internal_ms'] for r in rows),clock_calls=sorted({r['clock_calls'] for r in rows}),phases_ms={name:statistics.median(r['phases_ns'][name]/1e6 for r in rows) for name in rows[0]['phases_ns']},groups_ms={name:statistics.median(sum(r['phases_ns'][stage] for stage in stages)/1e6 for r in rows) for name,stages in groups.items()})
   record['summary'][variant]=summary
  assert len({row['output_sha256'] for row in record['summary'].values()})==1
  record['instrumentation_delta_ms']={v:record['summary']['profile_'+v]['whole_ms']-record['summary'][v]['whole_ms'] for v in ['current','reference']}
  result['cases'].append(record);(w/a.output).write_text(json.dumps(result,indent=2)+'\n');print(case,json.dumps(record['summary']),flush=True)
 for rep in range(4):
  c=subprocess.run([str(w/'clock_calibration')],check=True,stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True);prof=json.loads(c.stderr);assert prof['clock_calls']==1000001
  if rep:result['clock_calibration'].append(prof['internal_ns']/1000000)
result['clock_step_median_ns']=statistics.median(result['clock_calibration']);result['headers_unchanged']=all(sha(w/'source'/name)==digest for name,digest in result['metadata']['snapshot_sha256'].items());assert result['headers_unchanged'];(w/a.output).write_text(json.dumps(result,indent=2)+'\n')
