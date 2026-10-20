from pathlib import Path
import hashlib,json
HERE=Path(__file__).resolve().parent
if (HERE/'dense_native.json').exists() and (HERE/'remaining_native.json').exists():
    first=json.loads((HERE/'dense_native.json').read_text())
    last=json.loads((HERE/'remaining_native.json').read_text())
    assert first['flags']==last['flags'] and first['trials']==last['trials'] and first['source_hashes']==last['source_hashes']
    assert all(hashlib.sha256((HERE/name/path).read_bytes()).hexdigest()==digest for name,files in first['source_hashes'].items() for path,digest in files.items())
    first['samples'].update(last['samples'])
    first['segments']=['dense_native.json: official dense four cases completed; interrupted before synthetic case due to a checker-call TypeError','remaining_native.json: corrected checker-call; remaining five cases completed']
    first['validation']='all saved distances and warmup paths passed; source hashes unchanged'
    (HERE/'grouped_native.json').write_text(json.dumps(first,indent=2)+'\n')
out={}
for filename in ['grouped_native.json','grouped_generic.json','shuffled_native.json','emplace_native.json']:
    p=HERE/filename
    if not p.exists(): continue
    data=json.loads(p.read_text());table={}
    for case,result in data['samples'].items():
        row={}
        for variant in ['current','all','sorted','sample64','sample128','emplace']:
            key=variant+'_actual';phase=variant+'_phase'
            if key not in result['median']:continue
            row[variant]=dict(actual_ms=result['median'][key]['whole_ms'],build_ms=result['median'][phase]['build_ms'],phase_ms=result['median'][phase]['whole_ms'])
        table[case]=row
        print(filename,case, row,flush=True)
    out[filename]=table
(HERE/'summary.json').write_text(json.dumps(out,indent=2)+'\n')
