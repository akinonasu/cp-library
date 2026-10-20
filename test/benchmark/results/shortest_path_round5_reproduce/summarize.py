from pathlib import Path
import hashlib,json,statistics,subprocess,re,os
ROOT=Path(__file__).resolve().parent
out={}
for mode in ['native','generic']:
    path=ROOT/mode/'results.json'
    if not path.exists(): continue
    results=json.loads(path.read_text())
    env=json.loads((ROOT/mode/'environment.json').read_text())
    sources_unchanged={}
    for name,entry in env['sources'].items():
        paths=entry['manifest'] if 'manifest' in entry else {key:key for key in entry}
        hashes=entry['hashes'] if 'hashes' in entry else entry
        if 'manifest' in entry:
            revision=env['revisions'][name.removeprefix('own_')]
            checks=[]
            for key,digest in hashes.items():
                key=key.replace('\\','/')
                original=subprocess.check_output(['git','show',revision+':'+key]).replace(b'\r\n',b'\n').decode()
                assert hashlib.sha256(original.encode()).hexdigest()==digest
                def include(match):
                    target=os.path.normpath(str(Path(key).parent/match[2]))
                    if target not in paths: target=match[2]
                    return match[1]+paths[target]+match[3]
                expected=re.sub(r'(^\s*#include\s+")([^"\r\n]+)(")',include,original,flags=re.M)
                checks.append((ROOT/mode/'sources'/name/paths[key]).read_text()==expected)
            sources_unchanged[name]=all(checks)
        else:
            sources_unchanged[name]=all(hashlib.sha256((ROOT/mode/'sources'/name/paths[key]).read_bytes().replace(b'\r\n',b'\n')).hexdigest()==digest for key,digest in hashes.items())
    assert all(sources_unchanged.values())
    mode_out={'sources_unchanged':sources_unchanged,'cases':{}}
    for case,record in results.items():
        row={}
        for name,data in record['samples'].items():
            keys=['whole_ms','parse_ms','build_ms','solve_ms','output_ms','user_ms','system_ms','involuntary_switch','voluntary_switch']
            row[name]={key:dict(zip(['min','q1','median','q3','max'],[min(values),statistics.quantiles(values,n=4,method='inclusive')[0],statistics.median(values),statistics.quantiles(values,n=4,method='inclusive')[2],max(values)])) for key in keys if (values:=[item[key] for item in data if key in item])}
            if 'parse_ms' in data[0]:
                values=[item['parse_ms']+item['build_ms'] for item in data]
                row[name]['parse_build_ms']={'median':statistics.median(values)}
        mode_out['cases'][case]=row
        print(mode,case)
        for name,metrics in row.items():
            print(' ',name,' '.join(f'{key}={value["median"]:.4f}' for key,value in metrics.items()))
    out[mode]=mode_out
(ROOT/'summary.json').write_text(json.dumps(out,indent=2)+'\n')
