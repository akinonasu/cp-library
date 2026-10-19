from pathlib import Path
from collections import deque
import fcntl,json,hashlib,statistics,os
OUT=Path(__file__).resolve().parent
PROBLEM=Path.home()/'.cache/online-judge-tools/library-checker-problems/graph/shortest_path'
cases=list(json.loads((OUT/'native/results.json').read_text()))
results={}
os.sched_setaffinity(0,{2})
with open('/tmp/cp-wm-opt.lock','w') as lock:
    fcntl.flock(lock,fcntl.LOCK_EX)
    for case in cases:
        path=PROBLEM/'in'/(case+'.in')
        with path.open() as stream:
            n,m,source,target=map(int,next(stream).split())
            outgoing=[[] for _ in range(n)]
            incoming=[0]*n
            minimum=10**100;maximum=-1;zeros=0;self_edges=0;previous=-1;sorted_from=True
            for line in stream:
                u,v,w=map(int,line.split())
                outgoing[u].append(v);incoming[v]+=1
                minimum=min(minimum,w);maximum=max(maximum,w)
                zeros+=w==0;self_edges+=u==v
                sorted_from&=u>=previous;previous=u
        seen=bytearray(n);seen[source]=1
        pending=deque([source])
        while pending:
            for v in outgoing[pending.popleft()]:
                if not seen[v]: seen[v]=1;pending.append(v)
        degree=[len(row) for row in outgoing]
        row=dict(n=n,m=m,source=source,target=target,active_rows=sum(d!=0 for d in degree),min_outdegree=min(degree),max_outdegree=max(degree),min_indegree=min(incoming),max_indegree=max(incoming),min_weight=minimum,max_weight=maximum,zero_weights=zeros,self_edges=self_edges,nondecreasing_from=sorted_from,reachable_vertices=sum(seen),target_reachable=bool(seen[target]))
        samples=json.loads((OUT/'native/results.json').read_text())[case]['samples']['phase_c075']
        row['distance']=samples[0]['distance'];row['path_edges']=samples[0]['path_edges']
        assert all(s['distance']==row['distance'] and s['path_edges']==row['path_edges'] for s in samples)
        row['input_sha256']=hashlib.sha256(path.read_bytes()).hexdigest()
        results[case]=row
        print(case,row,flush=True)
(OUT/'features.json').write_text(json.dumps(results,indent=2)+'\n')
