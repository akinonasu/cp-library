import os, pathlib, subprocess, tempfile, random, time, json, statistics, fcntl, shutil, sys
W=pathlib.Path(__file__).resolve().parent
flags=['-std=gnu++23','-O2','-DNDEBUG','-DATCODER','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=native']
names=['old','current','deferred','build_noinline','records_noinline','run_noinline','path_noinline','queue_old']
memory=len(sys.argv)>1
if memory: names=['current','deferred','early_resize','early_resize_after_edges','early_reserve','prefetch_8_offset','prefetch_8_arc','prefetch_32_offset','prefetch_32_arc','prefetch_64_offset','prefetch_64_arc']
broader=len(sys.argv)>1 and sys.argv[1]=='broader'
if broader: names=['current','prefetch_8_arc','prefetch_32_arc','prefetch_64_arc']
combo=len(sys.argv)>1 and sys.argv[1]=='combo'
if combo: names=['current','prefetch_32_arc','guard32','deferred_guard32','narrow']
problem=pathlib.Path.home()/'.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
cases=['max_sparse_random_01','line_00','max_star_01','wrong_dijkstra_killer_00','grid_swirl_00']
if broader: cases=['max_dense_random_00','max_dense_zero_00','max_sparse_random_00','max_star_00','spfa_killer_00','wrong_dijkstra_killer_01','almost_line_00','grid_random_00']
if combo: cases=['max_sparse_random_01','line_00','max_star_01','max_dense_random_00','max_dense_zero_00','grid_swirl_00','wrong_dijkstra_killer_00']
os.sched_setaffinity(0,{2})
lock=open('/tmp/cp-wm-opt.lock','w')
with tempfile.TemporaryDirectory(prefix='cp-sp-codegen-') as td:
    tmp=pathlib.Path(td)
    binaries={n:tmp/n for n in names}
    for n in names:
        fcntl.flock(lock,fcntl.LOCK_EX)
        try: subprocess.run(['g++',*flags,str(W/n/'0.cpp'),'-o',str(binaries[n])],check=True)
        finally: fcntl.flock(lock,fcntl.LOCK_UN)
    print('Compiled',flush=True)
    results={}
    rng=random.Random(20261019)
    for case in cases:
        src=problem/'in'/(case+'.in')
        shutil.copyfile(src,tmp/'in')
        row={n:[] for n in names}; answers={}
        fcntl.flock(lock,fcntl.LOCK_EX)
        try:
            for trial in range(-1,7):
                order=names[:]; rng.shuffle(order)
                for name in order:
                    with (tmp/'in').open('rb') as inp, (tmp/'out').open('wb') as out:
                        begin=time.perf_counter()
                        subprocess.run([str(binaries[name])],stdin=inp,stdout=out,check=True)
                        ms=(time.perf_counter()-begin)*1000
                    if trial<0:
                        answers[name]=(tmp/'out').read_text().splitlines()[0].split()[0]
                    else: row[name].append(ms)
            assert len(set(answers.values()))==1,answers
        finally: fcntl.flock(lock,fcntl.LOCK_UN)
        results[case]={'samples':row,'median':{n:statistics.median(a) for n,a in row.items()}}
        (W/('combo.json' if combo else 'broader.json' if broader else 'memory.json' if memory else 'measure.json')).write_text(json.dumps(dict(flags=flags,trials=7,results=results),indent=2))
        print(case,results[case]['median'],flush=True)
