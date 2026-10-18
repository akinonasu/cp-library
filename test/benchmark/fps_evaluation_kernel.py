import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[2]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


harness='''
#include <chrono>
#include <cstdio>
#include <random>
using M=fps_detail::Mint;
volatile unsigned long long sink;
int main(int argc,char **argv){
 int op=atoi(argv[1]),n=atoi(argv[2]);
 std::vector<M>a(n),b(n);std::mt19937 rng(731);
 for(int i=0;i<n;i++) a[i]=M::raw(rng()%M::mod()),b[i]=M(i*7);
 for(int rep=0;rep<=atoi(argv[3]);rep++){
 auto begin=std::chrono::steady_clock::now();
 auto r=RUN(op,a,b);
 auto end=std::chrono::steady_clock::now();
 unsigned long long check=0;for(auto x:r)check=check*1000003+x.val();sink=check;
 if(rep)printf("%.9f %llu\\n",std::chrono::duration<double,std::milli>(end-begin).count(),check);
 }
}
'''
current='''#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
auto run(int op,const std::vector<fps_detail::Mint>&a,const std::vector<fps_detail::Mint>&b){
 if(op==0)return multipoint_eval(FormalPowerSeries<fps_detail::Mint>(a),b);
 return std::vector<fps_detail::Mint>(interpolate(b,a));
}
'''
adapter='''
#include "math/mint.hpp"
#include <vector>
std::vector<fps_detail::Mint> run(int op,const std::vector<fps_detail::Mint>&a,const std::vector<fps_detail::Mint>&b){
 using namespace poly;
 const size_t n=a.size();
 Arena arena((op?interpolate_words(n):evaluate_words(n,n))+3*Arena::footprint(n));
 auto f=arena.take(n),x=arena.take(n),y=arena.take(n);
 for(size_t i=0;i<n;i++) f[i]=a[i].val(),x[i]=b[i].val();
 if(op)interpolate(arena,x,f,y);else evaluate(arena,f,x,y);
 std::vector<fps_detail::Mint>r;r.reserve(n);for(auto v:y)r.emplace_back(fps_detail::Mint::raw(v));return r;
}
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--reference-root', type=Path, default=Path('/tmp/fps-reference'))
    parser.add_argument('--cpu', type=int, default=2)
    parser.add_argument('--n', type=int, default=131072)
    parser.add_argument('--trials', type=int, default=11)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    os.sched_setaffinity(0, {args.cpu})
    sources = {'current': current}
    for name, problem, sid in [('reference_eval', 'multipoint_evaluation', 409516),
                               ('reference_interpolate', 'polynomial_interpolation', 409545)]:
        source = (args.reference_root / problem / (str(sid) + '.cpp')).read_text()
        source = source[:source.index('// lib/run/early.hpp')] if sid == 409545 else source[:source.rindex('namespace {')]
        tail = adapter
        if sid == 409516:
            tail = tail.replace('(op?interpolate_words(n):evaluate_words(n,n))', 'evaluate_words(n,n)')
            tail = tail.replace('if(op)interpolate(arena,x,f,y);else evaluate(arena,f,x,y);', 'evaluate(arena,f,x,y);')
        sources[name] = source + tail
    flags = ['-std=c++23', '-O2', '-march=haswell', '-ftrivial-auto-var-init=zero']
    result = dict(environment=dict(cpu=args.cpu, n=args.n, trials=args.trials, flags=flags,
                                   harness_sha256=sha(Path(__file__)),
                                   compiler=subprocess.check_output(['g++', '--version'], text=True).splitlines()[0]),
                  sources={}, results=[])
    for name, code in sources.items():
        p = args.output / (name + '.cpp')
        p.write_text(code + harness.replace('RUN', 'run'))
        with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
            fcntl.flock(lock, fcntl.LOCK_SH)
            subprocess.run(['g++', *flags, '-I' + str(ROOT), str(p), '-o', str(args.output / name)], check=True)
        result['sources'][name] = dict(source_sha256=sha(p), binary_sha256=sha(args.output / name))
    with open('/tmp/cp-fps-tuning.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        for op, reference in [(0, 'reference_eval'), (1, 'reference_interpolate')]:
            checksums = set()
            for variant in ['current', reference]:
                raw = subprocess.check_output([str(args.output / variant), str(op), str(args.n), str(args.trials)], text=True)
                rows = [line.split() for line in raw.splitlines()]
                checksums.update(row[1] for row in rows)
                result['results'].append(dict(op=op, variant=variant, samples_ms=[float(row[0]) for row in rows],
                                              median_ms=statistics.median(float(row[0]) for row in rows), checksum=rows[0][1]))
                print(op, variant, result['results'][-1]['median_ms'], flush=True)
            assert len(checksums) == 1
    (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
