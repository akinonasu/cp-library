import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import urllib.parse
import urllib.request


CLASSIFICATION = {
    407097: dict(
        algorithm='Dijkstra with a circular 32-bit radix heap; full 64-bit fallback',
        separate_methods=['Topological relaxation of the zero-indegree removable part',
                          'Deterministic walk when all outdegrees or all indegrees are at most one'],
        queue=['Inline array for at most 32 records',
               'Cached minimum per nonzero radix bucket',
               'Packed low 32-bit distance and 32-bit vertex in each 64-bit record',
               '64-bit reconstructed extraction floor',
               '64-bit radix fallback if initial topological frontier spans more than 1e9'],
        constraints=['Input weights are at most 1e9',
                     'The circular queue requires live keys within floor..floor+1e9'],
        target_optimizations=['The target is not inserted into the queue',
                              'Stop when the minimum queued key is at least the discovered target distance',
                              'Non-target sinks are disabled'],
        graph='CSR', source_ranges={'queue': [99, 206], 'search': [208, 295]}),
    404592: dict(
        algorithm='Dijkstra with a 64-bit monotone radix heap',
        separate_methods=['Deterministic walk when all outdegrees or all indegrees are at most one'],
        queue=['Inline array for at most 32 records',
               'Bucket zero stores only vertex IDs',
               'Occupied-bucket bitmask',
               'Singleton nonzero bucket is returned directly',
               'AVX2 minimum reduction for buckets containing at least 8 records',
               'uint64 key and uint32 vertex use a 16-byte record'],
        target_optimizations=['The target is not inserted into the queue',
                              'Stop when the minimum queued key is at least the discovered target distance',
                              'Non-target sinks are disabled'],
        graph='CSR', source_ranges={'minimum': [2591, 2679], 'queue': [2894, 3095],
                                    'search': [3210, 3259]}),
    403027: dict(
        algorithm='Dijkstra with a 64-bit monotone radix heap',
        same_search_and_queue_as=404592,
        separate_methods=['Deterministic walk when all outdegrees or all indegrees are at most one'],
        graph='CSR'),
    410532: dict(algorithm='Reverse Dijkstra with a two-level integer bucket heap',
                 separate_methods=['Reverse deterministic walk when every indegree is at most one'],
                 queue=['18-bit lower level and circular 12-bit upper level',
                        'Hierarchical occupancy bitsets', 'Inline array for at most 32 records'],
                 constraints=['Input weights are at most 1e9', 'Fixed capacities for at most 500000 edges'],
                 graph='Linked edge arrays'),
    386103: dict(algorithm='Dijkstra with a 64-bit radix heap',
                 separate_methods=['Deterministic walk when every outdegree is at most one'],
                 target_optimizations=['Non-target sinks are disabled', 'Stop when the target is extracted'],
                 graph='CSR'),
    410434: dict(algorithm='Dijkstra with a two-level integer bucket heap',
                 separate_methods=[], graph='Linked edge arrays', queue_variant=410532),
    410523: dict(algorithm='Reverse Dijkstra with a two-level integer bucket heap',
                 separate_methods=[], graph='Linked edge arrays', queue_variant=410532),
    410522: dict(algorithm='Dijkstra with a two-level integer bucket heap',
                 separate_methods=[], graph='CSR', queue_variant=410532),
    386101: dict(algorithm='Dijkstra with a 64-bit radix heap',
                 separate_methods=['Deterministic walk when every outdegree is at most one'],
                 target_optimizations=['Non-target sinks are disabled', 'Stop when the target is extracted'],
                 graph='CSR'),
    410428: dict(algorithm='Dijkstra with a two-level integer bucket heap',
                 separate_methods=[], graph='Linked edge arrays', queue_variant=410431),
    373581: dict(algorithm='Reverse Dijkstra with a custom binary heap',
                 separate_methods=[], graph='Linked edge arrays',
                 queue=['Hole-based heap movement', 'Explicit settled flags', 'Zero means unreached; distance stored plus one']),
    373573: dict(algorithm='Reverse Dijkstra with a custom binary heap',
                 separate_methods=[], graph='Linked edge arrays',
                 queue=['Hole-based heap movement', 'Zero means unreached; distance stored plus one']),
    410431: dict(algorithm='Dijkstra with a two-level integer bucket heap',
                 separate_methods=[], graph='Linked edge arrays',
                 queue=['18-bit lower level and circular 12-bit upper level',
                        'Hierarchical occupancy bitsets', 'No inline array'],
                 constraints=['Input weights are at most 1e9', 'Fixed capacities for at most 500000 edges']),
    210465: dict(algorithm='Reverse Dijkstra with a binary heap', separate_methods=[], graph='CSR',
                 target_optimizations=['None; all reachable vertices are processed']),
    372228: dict(algorithm='Reverse Dijkstra with a binary heap', separate_methods=[], graph='CSR',
                 target_optimizations=['None; all reachable vertices are processed']),
    305673: dict(algorithm='Reverse Dijkstra with an indexed 16-ary heap',
                 separate_methods=['BFS when all weights are one', 'Degree-checked path walk'], graph='CSR'),
    327722: dict(algorithm='Reverse Dijkstra with an indexed 16-ary heap',
                 separate_methods=['BFS when all weights are one', 'Degree-checked path walk'], graph='CSR'),
    305668: dict(algorithm='Reverse Dijkstra with an indexed 8-ary heap',
                 separate_methods=['BFS when all weights are one', 'Degree-checked path walk'], graph='CSR'),
    291460: dict(algorithm='Reverse Dijkstra with a 4-ary heap',
                 separate_methods=['BFS when all weights are one', 'Degree-checked path walk'], graph='CSR'),
    336438: dict(algorithm='Reverse Dijkstra with a binary heap', separate_methods=[], graph='CSR',
                 target_optimizations=['None; all reachable vertices are processed']),
}


COMPARISON = dict(
    scope='The general-graph search in every inspected top-20 submission is Dijkstra. Some have separate special-case algorithms.',
    reusable_candidates=[
        'Small-queue linear minimum before radix migration',
        'Bucket-zero vertex-only storage',
        'Direct extraction from a singleton bucket',
        'Occupied-bucket bitmask',
        'Cached minima versus SIMD minimum scans',
        'CSR graph storage',
        'Single-target upper-bound termination',
    ],
    queue_tradeoffs={
        'cached_minimum': '407097 updates one minimum during insertion and redistribution, avoiding a separate minimum scan at refill.',
        'simd_scan': '404592 scans record keys at refill, uses scalar reduction below 8 records and AVX2 otherwise.',
        'circular_32bit': '407097 packs key and vertex into 8 bytes. Correct ordering depends on a bounded live-key window; its topological initial frontier can require a 64-bit fallback.',
        'general_64bit': '404592 accepts monotone unsigned 64-bit keys without a maximum-edge bound. Its key/vertex records occupy 16 bytes and bucket zero stores only vertices.',
        'inline': 'All three use a linear minimum for at most 32 pending records and migrate permanently once that limit is crossed.',
    },
    target_contract=[
        'A finite tentative target distance is an upper bound on the optimal target distance.',
        'If every queued key is at least that bound, nonnegative edges cannot improve it.',
        'The target need not enter the queue.',
        'Suppressing other sinks and stopping early do not preserve final distances to all vertices.',
        'Apply these optimizations only to an explicitly single-target API, preserving the all-distances contract.',
    ],
    separate_algorithm_policy='Keep the default Dijkstra API; enable topological preprocessing only through the explicit dijkstra<true> and dijkstra_path<true> extension. No deterministic-walk or BFS branch is included.',
    measurement_caveat='API times include parsing, graph construction, path output, and special-case branches; they do not isolate the Dijkstra kernel.',
    judge_environment='The public submission API returns lang=cpp and case timings, but does not identify the historical compiler version, flags, or CPU. Do not infer local performance from API rank.',
    license='No explicit license grant was found in the inspected submission sources. Sources remain in the ignored reference cache; compare algorithmic ideas and independently implement repository code.',
)


def get(url):
    request = urllib.request.Request(url, headers={'User-Agent': 'cp-library-shortest-path'})
    with urllib.request.urlopen(request, timeout=90) as response:
        return json.load(response)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cache', type=Path, default=Path('.verify-helper/cache/shortest-path/reference'))
    parser.add_argument('--output', type=Path, default=Path('test/benchmark/results/shortest_path_reference.json'))
    parser.add_argument('--ids', type=int, nargs='+')
    parser.add_argument('--limit', type=int, default=20)
    parser.add_argument('--offline', action='store_true')
    args = parser.parse_args()
    args.cache.mkdir(parents=True, exist_ok=True)
    query = urllib.parse.urlencode(dict(problem='shortest_path', status='AC', order='+time', limit=args.limit))
    url = 'https://v3.api.judge.yosupo.jp/submissions?' + query
    listing_path = args.cache / 'listing.json'
    if args.offline:
        listing = json.loads(listing_path.read_text())
    else:
        listing = get(url)
        listing_path.write_text(json.dumps(listing, indent=2) + '\n')
    ids = args.ids or [row['id'] for row in listing['submissions']]
    references = []
    for identity in dict.fromkeys(ids):
        detail_path = args.cache / f'{identity}.json'
        if args.offline:
            detail = json.loads(detail_path.read_text())
        else:
            detail = get(f'https://v3.api.judge.yosupo.jp/submissions/{identity}')
            detail_path.write_text(json.dumps(detail, indent=2) + '\n')
        source = detail['source'].encode()
        (args.cache / f'{identity}.cpp').write_bytes(source)
        rank = next((i + 1 for i, row in enumerate(listing['submissions']) if row['id'] == identity), None)
        references.append(dict(**detail['overview'], rank=rank,
                               source_sha256=hashlib.sha256(source).hexdigest(),
                               source_bytes=len(source), classification=CLASSIFICATION.get(identity),
                               case_results=detail['case_results'],
                               url=f'https://judge.yosupo.jp/submission/{identity}'))
        print(identity, detail['overview'].get('user_name', detail.get('user')), detail['overview']['time'], flush=True)
    result = dict(retrieved_at=datetime.now(timezone.utc).isoformat(), problem='shortest_path',
                  ranking='All AC submissions ordered by +time; users are not deduplicated',
                  listing_url=url, listing=listing, references=references, comparison=COMPARISON)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
