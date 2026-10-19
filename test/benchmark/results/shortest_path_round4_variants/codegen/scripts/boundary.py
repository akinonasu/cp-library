import os, pathlib, subprocess, tempfile, random, time, json, statistics, fcntl, sys
W=pathlib.Path(__file__).resolve().parent
flags=['-std=gnu++23','-O2','-DNDEBUG','-DATCODER','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=native']
names=['current','prefetch_32_arc']
os.sched_setaffinity(0,{2})
lock=open('/tmp/cp-wm-opt.lock','w')
source='''#include "6.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>
int main(int argc, char **argv) {
    const int n = std::atoi(argv[1]), degree = std::atoi(argv[2]);
    using Graph = DirectedCSR<unsigned>;
    std::mt19937 rng(20261019);
    std::vector<Graph::Edge> edges;
    edges.reserve(std::size_t(n) * degree);
    for (int i = 0; i < n * degree; ++i) edges.push_back({int(rng() % n), int(rng() % n), unsigned(rng() % 1000000000)});
    std::size_t sum = 0;
    for (int trial = -2; trial < 31; ++trial) {
        const auto start = std::chrono::steady_clock::now();
        Graph g(n, edges);
        const auto end = std::chrono::steady_clock::now();
        for (int v = 0; v < n; ++v) sum += g[v].size();
        if (trial >= 0) std::printf("%.9f\\n", std::chrono::duration<double, std::milli>(end-start).count());
    }
    return sum != edges.size() * 33;
}
'''
with tempfile.TemporaryDirectory(prefix='cp-sp-boundary-') as td:
    tmp=pathlib.Path(td); binaries={}
    for name in names:
        (W/name/'boundary.cpp').write_text(source)
        binary=tmp/name; binaries[name]=binary
        fcntl.flock(lock,fcntl.LOCK_EX)
        try: subprocess.run(['g++',*flags,str(W/name/'boundary.cpp'),'-o',str(binary)],check=True)
        finally: fcntl.flock(lock,fcntl.LOCK_UN)
    results={}; rng=random.Random(20261019)
    lower=len(sys.argv)>1
    for n in ((512,1024,2048) if lower else (4096,8192,16384,32768,65536)):
        for degree in (2,16):
            row={name:[] for name in names}
            fcntl.flock(lock,fcntl.LOCK_EX)
            try:
                for trial in range(3):
                    order=names[:]; rng.shuffle(order)
                    for name in order:
                        values=subprocess.check_output([str(binaries[name]),str(n),str(degree)],text=True)
                        row[name].append([float(value) for value in values.splitlines()])
            finally: fcntl.flock(lock,fcntl.LOCK_UN)
            median={name:statistics.median([value for sample in data for value in sample]) for name,data in row.items()}
            results[f'{n}_{degree}']=dict(n=n,m=n*degree,degree=degree,samples_ms=row,median_ms=median)
            (W/('boundary_lower.json' if lower else 'boundary.json')).write_text(json.dumps(dict(flags=flags,seed=20261019,warmup_per_process=2,measurements_per_process=31,processes=3,method='CSR span construction only; input generation outside timing; graph destruction/checksum outside timing; allocator/cache warm; randomized process order; CPU2; shared lock',results=results),indent=2))
            print(n,degree,median,flush=True)
