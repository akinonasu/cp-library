import hashlib
import json
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[5]
OUT = ROOT / 'test/benchmark/results/shortest_path_round4_variants/candidate_a'
OUT.mkdir(parents=True, exist_ok=True)


def sha(path):
    return hashlib.sha256(path.read_bytes().replace(b'\r\n', b'\n')).hexdigest()


def stats(values):
    quartiles = statistics.quantiles(values, n=4, method='inclusive')
    return dict(n=len(values), minimum=min(values), q1=quartiles[0], median=statistics.median(values), q3=quartiles[2], maximum=max(values))


result = dict(candidate='A', description='bd29f70 + deferred incoming check + 32-edge prefetch when vertex count >= 8192; superseded candidate under investigation because of max_star_00', baseline='bd29f70c70e24e27770b9e9a3777b6cf85664b63', quartile_method='Python statistics.quantiles inclusive (R type 7)', settings={})
for setting in ('native', 'generic'):
    directory = ROOT / ('test/benchmark/results/shortest_path_round4_' + setting)
    environment = json.loads((directory / 'environment.json').read_text())
    records = json.loads((directory / 'results.json').read_text())
    assert environment.get('completed_at') and environment['validation']['cases'] == 36
    assert environment['validation']['distances'] == environment['validation']['warmup_paths'] == 'passed'
    assert environment['trials'] == 21 and len(records) == 36
    assert all(sha(directory / key) == value for key, value in environment['archived_sources'].items())
    rows = {}
    for name, record in records.items():
        assert all(len(samples) == 21 for samples in record['samples'].values())
        actual = {variant: stats([sample['whole_ms'] for sample in record['samples'][variant]]) for variant in ('own_baseline', 'own_path', 'own_topo')}
        before = actual['own_baseline']['median']
        after = actual['own_path']['median']
        phases = {}
        for variant in ('baseline', 'current'):
            phases[variant] = {phase: stats([sample[phase] for sample in record['samples'][variant]]) for phase in ('parse_ms', 'build_ms', 'solve_ms', 'output_ms')}
            phases[variant]['input_build_ms'] = stats([sample['parse_ms'] + sample['build_ms'] for sample in record['samples'][variant]])
        rows[name] = dict(n=record['n'], m=record['m'], actual=actual, phases=phases, change_ms=after-before, change_percent=100*(after/before-1), input_sha256=record['input_sha256'])
    sparse = records['max_sparse_random_01']
    histogram = {}
    for variant in ('own_baseline', 'own_path'):
        values = [sample['whole_ms'] for sample in sparse['samples'][variant]]
        histogram[variant] = {label: sum(low <= value < high for value in values) for label, low, high in (('[0,12)',0,12),('[12,14)',12,14),('[14,16)',14,16),('[16,+inf)',16,float('inf')))}
    result['settings'][setting] = dict(source_environment=directory.relative_to(ROOT).as_posix() + '/environment.json', source_results=directory.relative_to(ROOT).as_posix() + '/results.json', environment_sha256=sha(directory / 'environment.json'), results_sha256=sha(directory / 'results.json'), csr_sha256=environment['sources']['current']['components/directed_csr.hpp'], completed_at=environment['completed_at'], rows=rows, sparse_histogram_ms=histogram, maximum_absolute_increase=max(rows, key=lambda key: rows[key]['change_ms']), maximum_percent_increase=max(rows, key=lambda key: rows[key]['change_percent']), maximum_percent_increase_over_5ms=max((key for key in rows if rows[key]['actual']['own_baseline']['median'] >= 5), key=lambda key: rows[key]['change_percent']))
(OUT / 'summary.json').write_text(json.dumps(result, indent=2) + '\n')

lines = ['# 候補 A の全 36 ケース測定', '', '最終採用版の表ではありません。`bd29f70` に、入次数判定の後回しと、頂点数 8192 以上での 32 辺先読みを加えた候補 A です。`max_star_00` の構築が両設定で悪化したため、先読みの適用条件を追加調査しています。', '', 'Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定、共有 lock、各 1 回 warmup 後 21 回、順序を変えた fresh process の中央値です。単位は ms。入力・構築・探索・出力・プロセス起動終了を含む実提出 `own_baseline` と `own_path` を比較します。区間ドライバの全体時間は使いません。`own_topo` は別 API の結果として JSON のみに残します。', '', '| ケース | native 前 | native A | 変化 | generic 前 | generic A | 変化 |', '| --- | ---: | ---: | ---: | ---: | ---: | ---: |']
for name in result['settings']['native']['rows']:
    vals = []
    for setting in ('native','generic'):
        row=result['settings'][setting]['rows'][name]
        vals.extend([f"{row['actual']['own_baseline']['median']:.3f}", f"{row['actual']['own_path']['median']:.3f}", f"{row['change_percent']:+.1f}%"])
    lines.append('| `'+name+'` | '+' | '.join(vals)+' |')
lines += ['', '## 疎グラフの分布', '', '`max_sparse_random_01` の全体時間です。四分位数は inclusive（R type 7）です。', '', '| 設定・実装 | 最小 | Q1 | 中央値 | Q3 | 最大 |', '| --- | ---: | ---: | ---: | ---: | ---: |']
for setting in ('native','generic'):
    for variant,label in (('own_baseline','前'),('own_path','A')):
        row=result['settings'][setting]['rows']['max_sparse_random_01']['actual'][variant]
        lines.append('| '+setting+' '+label+' | '+' | '.join(f'{row[key]:.3f}' for key in ('minimum','q1','median','q3','maximum'))+' |')
lines += ['', '| 設定・実装 | 12ms 未満 | 12–14ms | 14–16ms | 16ms 以上 |', '| --- | ---: | ---: | ---: | ---: |']
for setting in ('native','generic'):
    for variant,label in (('own_baseline','前'),('own_path','A')):
        row=result['settings'][setting]['sparse_histogram_ms'][variant]
        lines.append('| '+setting+' '+label+' | '+' | '.join(str(value) for value in row.values())+' |')
lines += ['', '## 区間測定', '', '各試行の入力＋構築を足してから中央値を取ります。採時と stderr JSON を含む区間ドライバなので、実提出の全体時間とは分けて読んでください。', '', '| 設定・ケース | 入力＋構築 前 | 入力＋構築 A | 計算 前 | 計算 A | 出力 前 | 出力 A |', '| --- | ---: | ---: | ---: | ---: | ---: | ---: |']
for setting in ('native','generic'):
    for name in ('max_sparse_random_01','line_00','max_star_00','max_star_01','grid_swirl_00','wrong_dijkstra_killer_00'):
        row=result['settings'][setting]['rows'][name]['phases']; cells=[]
        for phase in ('input_build_ms','solve_ms','output_ms'):
            cells += [f"{row[variant][phase]['median']:.3f}" for variant in ('baseline','current')]
        lines.append('| '+setting+' `'+name+'` | '+' | '.join(cells)+' |')
lines += ['', '## 残る悪化', '']
for setting in ('native','generic'):
    data=result['settings'][setting]
    for category,label in (('maximum_absolute_increase','最大の増分'),('maximum_percent_increase','最大の割合'),('maximum_percent_increase_over_5ms','変更前が 5ms 以上での最大の割合')):
        name=data[category]; row=data['rows'][name]
        lines.append(f"- {setting}、{label}: `{name}` {row['actual']['own_baseline']['median']:.3f} → {row['actual']['own_path']['median']:.3f} ms、{row['change_ms']:+.3f} ms、{row['change_percent']:+.1f}%。")
lines += ['', '小さい入力の全体時間はプロセス起動・終了などの固定費が支配的です。大きな相対割合だけをアルゴリズムの退行とは扱いません。`max_star_00` は構築区間にも悪化が出たため、別途改善を進めます。', '', '[集計 JSON](summary.json)、[native 全試行](../../shortest_path_round4_native/results.json)、[native 環境・凍結ソース](../../shortest_path_round4_native/environment.json)、[generic 全試行](../../shortest_path_round4_generic/results.json)、[generic 環境・凍結ソース](../../shortest_path_round4_generic/environment.json) を参照してください。両設定の全距離・warmup 経路・測定終了時の凍結ソース hash の照合は成功しました。', '']
(OUT / 'README.md').write_text('\n'.join(lines))
print(json.dumps({setting: {key: value for key,value in data.items() if key not in ('rows',)} for setting,data in result['settings'].items()},indent=2))
