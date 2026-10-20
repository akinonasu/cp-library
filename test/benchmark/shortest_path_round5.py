import random

from shortest_path import synthetic_cases
from shortest_path_round3 import main


def general_cases(destination):
    cases = synthetic_cases(destination)
    rng = random.Random(2026101905)

    def write(name, n, edges, source=0, target=None):
        vertices = list(range(n))
        rng.shuffle(vertices)
        source, target = vertices[source], vertices[n-1 if target is None else target]
        edges = [(vertices[u], vertices[v], w) for u, v, w in edges]
        rng.shuffle(edges)
        path = destination / (name + '.in')
        with path.open('w') as stream:
            stream.write(f'{n} {len(edges)} {source} {target}\n')
            stream.writelines(f'{u} {v} {w}\n' for u, v, w in edges)
        cases.append(path)

    n = 707
    write('synthetic_dense_shuffled', n,
          [(u, v, rng.randrange(10**9)) for u in range(n) for v in range(n) if u != v])
    n, m = 200000, 500000
    edges = [(v-1, v, rng.randrange(10**9)) for v in range(1, n)]
    edges += [(rng.randrange(n), rng.randrange(n), rng.randrange(10**9)) for _ in range(m-len(edges))]
    write('synthetic_sparse_shuffled', n, edges)
    edges = [(v-1, v, rng.randrange(10**9)) for v in range(1, n)]
    hubs = rng.sample(range(n), 8)
    edges += [(rng.choice(hubs) if rng.randrange(4) else rng.randrange(n),
               rng.randrange(n), rng.randrange(10**9)) for _ in range(m-len(edges))]
    write('synthetic_skew_shuffled', n, edges)
    n = 500000
    write('synthetic_line_permuted', n,
          [(v-1, v, 10**9) for v in range(1, n)])
    n = 250000
    hub = rng.randrange(n)
    edges = [(hub, v, rng.randrange(10**9)) for v in range(n) if v != hub]
    edges += [(v, hub, rng.randrange(10**9)) for v in range(n) if v != hub]
    write('synthetic_star_shuffled', n, edges, hub, (hub+1) % n)
    return cases


if __name__ == '__main__':
    main(baseline='c075fb733c2954ce303eb6a7fa1ef78977a611f9', driver_path='test/benchmark/shortest_path_round4.cpp',
         cache_name='round5', entrypoint=__file__, synthetic_factory=general_cases)
