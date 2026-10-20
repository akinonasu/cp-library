from pathlib import Path
import fcntl,json,statistics,random,os
OUT=Path(__file__).resolve().parent
out={};rng=random.Random(2026101905)
os.sched_setaffinity(0,{2})
with open('/tmp/cp-wm-opt.lock','w') as lock:
    fcntl.flock(lock,fcntl.LOCK_EX)
    for mode in ['native','generic']:
        records=json.loads((OUT/mode/'results.json').read_text())
        out[mode]={}
        for case in ['max_dense_random_01','max_dense_random_00','max_dense_zero_00','max_dense_long_00','max_sparse_random_01']:
            record=records[case]['samples'];row={}
            for prefix in ['own_','phase_']:
                before=[s['whole_ms'] for s in record[prefix+'bd']]
                after=[s['whole_ms'] for s in record[prefix+'c075']]
                values=[]
                for rep in range(10000):
                    indices=rng.choices(range(len(before)),k=len(before))
                    values.append(statistics.median(after[i] for i in indices)/statistics.median(before[i] for i in indices)-1)
                values.sort()
                row[prefix.removesuffix('_')]=dict(change=statistics.median(after)/statistics.median(before)-1,bootstrap95=[values[249],values[9749]])
            out[mode][case]=row
            print(mode,case,row,flush=True)
(OUT/'intervals.json').write_text(json.dumps(dict(seed=2026101905,method='paired by trial; 10000 bootstrap resamples; percentile 95% interval of ratio of medians',results=out),indent=2)+'\n')
