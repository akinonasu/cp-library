import argparse
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import math
import os
from pathlib import Path
import random
import statistics


ROOT = Path(__file__).resolve().parents[2]
KEY_CASES = [
    'max_dense_random_01', 'max_dense_random_00', 'max_sparse_random_01', 'line_00',
    'synthetic_dense_shuffled', 'synthetic_sparse_shuffled', 'synthetic_skew_shuffled',
    'synthetic_line_permuted', 'synthetic_star_shuffled',
]


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def five(values):
    quartiles = statistics.quantiles(values, n=4, method='inclusive')
    return dict(n=len(values), minimum=min(values), q1=quartiles[0], median=statistics.median(values),
                q3=quartiles[2], maximum=max(values))


def change(before, after):
    assert before > 0
    return dict(before_ms=before, after_ms=after, delta_ms=after-before,
                ratio=after/before, change_percent=100*(after/before-1))


def percentile(values, probability):
    position = (len(values)-1)*probability
    low = math.floor(position)
    high = math.ceil(position)
    return values[low] + (values[high]-values[low])*(position-low)


def bootstrap(before, after, seed, repetitions):
    assert len(before) == len(after)
    rng = random.Random(seed)
    ratios = []
    for _ in range(repetitions):
        indices = rng.choices(range(len(before)), k=len(before))
        ratios.append(statistics.median(after[i] for i in indices) /
                      statistics.median(before[i] for i in indices))
    ratios.sort()
    interval = [percentile(ratios, 0.025), percentile(ratios, 0.975)]
    return dict(seed=seed, resamples=repetitions, ratio=statistics.median(after)/statistics.median(before),
                ratio95=interval, change_percent95=[100*(value-1) for value in interval])


def load(directory, expected_cases, expected_trials):
    environment = json.loads((directory / 'environment.json').read_text())
    results = json.loads((directory / 'results.json').read_text())
    assert environment.get('completed_at'), str(directory) + ': benchmark incomplete'
    validation = environment.get('validation', {})
    assert validation.get('cases') == expected_cases == len(results)
    assert validation.get('distances') == 'passed'
    assert validation.get('warmup_paths') == 'passed'
    assert validation.get('live_sources_unchanged') is True
    assert validation.get('archived_sources_unchanged') is True
    assert environment['trials'] == expected_trials
    variants = set(environment['sources']) | set(environment['references'])
    for name in ['baseline', 'current', 'own_baseline', 'own_path', 'own_topo']:
        assert name in variants
    for case, record in results.items():
        assert set(record['samples']) == variants, case
        assert 0 <= record['source'] < record['n'] and 0 <= record['target'] < record['n']
        for name, samples in record['samples'].items():
            assert len(samples) == expected_trials, (case, name)
            assert all(row['distance'] == record['distance'] for row in samples), (case, name)
            assert all(math.isfinite(row['whole_ms']) and row['whole_ms'] > 0 for row in samples)
    assert all(sha(ROOT / path) == digest for path, digest in environment['live_sources'].items())
    assert all(sha(directory / path) == digest for path, digest in environment['archived_sources'].items())
    for path, key in [('test/benchmark/shortest_path_round4.cpp', 'driver_sha256'),
                      ('test/benchmark/shortest_path_round5.py', 'runner_sha256'),
                      ('test/benchmark/shortest_path_round3.py', 'helper_sha256'),
                      ('test/benchmark/shortest_path.py', 'utility_sha256')]:
        assert sha(ROOT / path) == environment[key], path
    return environment, results


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--native', type=Path, default=ROOT/'test/benchmark/results/shortest_path_round5_native')
    parser.add_argument('--generic', type=Path, default=ROOT/'test/benchmark/results/shortest_path_round5_generic')
    parser.add_argument('--output', type=Path, default=ROOT/'test/benchmark/results/shortest_path_round5_summary.json')
    parser.add_argument('--expected-cases', type=int, default=41)
    parser.add_argument('--trials', type=int, default=21)
    parser.add_argument('--seed', type=int, default=2026101905)
    parser.add_argument('--resamples', type=int, default=10000)
    parser.add_argument('--cpu', type=int, default=2)
    args = parser.parse_args()
    os.sched_setaffinity(0, {args.cpu})
    with open('/tmp/cp-wm-opt.lock', 'w') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        loaded = {name: load(directory.resolve(), args.expected_cases, args.trials)
                  for name, directory in [('native', args.native), ('generic', args.generic)]}
        native_environment, native_results = loaded['native']
        generic_environment, generic_results = loaded['generic']
        assert native_environment['baseline'] == generic_environment['baseline']
        for key in ['sources', 'live_sources', 'archived_sources', 'driver_sha256', 'runner_sha256',
                    'helper_sha256', 'utility_sha256']:
            assert native_environment[key] == generic_environment[key], key
        assert set(native_results) == set(generic_results)
        assert all(native_results[case]['input_sha256'] == generic_results[case]['input_sha256']
                   for case in native_results)
        output = dict(created_at=datetime.now(timezone.utc).isoformat(),
                      baseline=native_environment['baseline'], source_sha256=native_environment['sources']['current'],
                      summary_script_sha256=sha(Path(__file__)),
                      quartile_method='Python statistics.quantiles inclusive (R type 7)',
                      bootstrap_method='paired by trial; ratio of medians; percentile 95% interval using R type 7; independent fixed seed per setting/case/kind',
                      base_seed=args.seed, resamples=args.resamples, key_cases=KEY_CASES,
                      validation=dict(completed_runs=True, cases=args.expected_cases, trials=args.trials,
                                      same_inputs=True, sources_match_between_settings=True,
                                      live_sources_match=True, archived_sources_match=True), settings={})
        for setting, directory in [('native', args.native.resolve()), ('generic', args.generic.resolve())]:
            environment, results = loaded[setting]
            rows = {}
            for case, record in sorted(results.items()):
                actual = {name: five([sample['whole_ms'] for sample in samples])
                          for name, samples in record['samples'].items() if name in environment['references']}
                phases = {}
                for name in environment['sources']:
                    samples = record['samples'][name]
                    phases[name] = {key: five([sample[key] for sample in samples])
                                    for key in ['parse_ms', 'build_ms', 'solve_ms', 'output_ms', 'whole_ms']}
                    phases[name]['parse_build_ms'] = five([sample['parse_ms']+sample['build_ms'] for sample in samples])
                row = dict(n=record['n'], m=record['m'], source=record['source'], target=record['target'],
                           input_sha256=record['input_sha256'], distance=record['distance'], actual=actual, phases=phases,
                           actual_change=change(actual['own_baseline']['median'], actual['own_path']['median']),
                           phase_change={key: change(phases['baseline'][key]['median'], phases['current'][key]['median'])
                                         for key in phases['baseline']})
                if case in KEY_CASES:
                    row['bootstrap'] = {}
                    for kind, before_name, after_name in [('actual_whole', 'own_baseline', 'own_path'),
                                                          ('phase_whole', 'baseline', 'current')]:
                        seed = args.seed ^ int(hashlib.sha256((setting+'/'+case+'/'+kind).encode()).hexdigest()[:16], 16)
                        row['bootstrap'][kind] = bootstrap(
                            [sample['whole_ms'] for sample in record['samples'][before_name]],
                            [sample['whole_ms'] for sample in record['samples'][after_name]], seed, args.resamples)
                rows[case] = row
            worst = {}
            for name, selected in [('all', list(rows)), ('baseline_at_least_5ms', [case for case, row in rows.items()
                                                                              if row['actual_change']['before_ms'] >= 5])]:
                worst[name] = dict(cases=len(selected))
                for metric in ['delta_ms', 'change_percent']:
                    case = max(selected, key=lambda case: rows[case]['actual_change'][metric])
                    worst[name][metric] = dict(case=case, **rows[case]['actual_change'])
            output['settings'][setting] = dict(environment_sha256=sha(directory/'environment.json'),
                                               results_sha256=sha(directory/'results.json'), flags=environment['flags'],
                                               rows=rows, worst=worst)
            print(setting, 'case actual_before actual_after change_percent build_before build_after', flush=True)
            for case in KEY_CASES:
                row = rows[case]
                print(case, *(round(row['actual_change'][key], 4) for key in ['before_ms', 'after_ms', 'change_percent']),
                      round(row['phases']['baseline']['build_ms']['median'], 4),
                      round(row['phases']['current']['build_ms']['median'], 4), flush=True)
            print(setting, 'worst', worst, flush=True)
        for setting, directory in [('native', args.native.resolve()), ('generic', args.generic.resolve())]:
            saved = output['settings'][setting]
            assert sha(directory/'environment.json') == saved['environment_sha256']
            assert sha(directory/'results.json') == saved['results_sha256']
        assert all(sha(ROOT/path) == digest for path, digest in native_environment['live_sources'].items())
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(output, indent=2)+'\n')


if __name__ == '__main__':
    main()
