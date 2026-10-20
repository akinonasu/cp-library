# 先読みする行数の比較

初回の全ケース測定で `max_star_00` に悪化が出たため、先読みする条件と入次数判定を比較しました。基準は `bd29f70` です。`current` はこの予備比較時の実装で、最終実装を意味しません。

| 名前 | 入次数の判定 | 先読み条件 |
| --- | --- | --- |
| `bd` | 辺の入力中 | なし |
| `current` | 入力後に辺を走査 | 頂点数が 8192 以上 |
| `active` | 入力後に辺を走査 | 出辺がある頂点数が 8192 以上 |
| `deferred` | 入力後に辺を走査 | なし |
| `original_active` | 辺の入力中 | 出辺がある頂点数が 8192 以上 |

出辺がある頂点数は次数の累積和を作るループで数えます。追加の頂点配列は使いません。`max_star_00` は頂点数が 500000 ですが、出辺がある頂点は 32 です。頂点数だけで選ぶと、少数の行に繰り返し先読みを発行します。新しい条件もグラフの構造だけから選び、頂点番号・重み・入力名は使いません。

WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2、CPU affinity 2 です。`-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=native` を使い、共有 lock `/tmp/cp-wm-opt.lock` の下でコンパイル・測定しました。バイナリ・入出力は Linux `/tmp`、warmup 1 回・計測 9 回、毎回新しいプロセスで実行し、seed `2026101910` で試行順を変えました。各ケース・各 driver の warmup は公式 checker または合成データの辺による検証に通し、全試行の距離を照合しました。

[実提出・区間ドライバの全試行](results.json)、[環境・凍結時の hash](environment.json)、[最終保存ファイルの hash とパッチ検証](manifest.json)です。実提出の全体時間の中央値を比較します。単位は ms です。

| ケース | `bd` | `current` | `active` | `deferred` | `original_active` |
| --- | ---: | ---: | ---: | ---: | ---: |
| `max_star_00` | 8.979 | 9.164 | 9.175 | 8.999 | 8.785 |
| `max_star_01` | 9.292 | 8.698 | 9.174 | 9.257 | 9.137 |
| `max_sparse_random_01` | 14.929 | 13.133 | 13.181 | 13.831 | 12.918 |
| `line_00` | 20.667 | 20.354 | 19.811 | 19.941 | 20.509 |
| `grid_swirl_00` | 15.846 | 15.013 | 14.661 | 15.659 | 14.756 |
| `max_dense_random_00` | 6.123 | 6.464 | 6.448 | 6.463 | 6.234 |
| `synthetic_line` | 15.398 | 14.372 | 13.820 | 15.451 | 13.646 |

`max_star_00` の区間ドライバでは次の結果でした。採時・JSON 出力を含むため、実提出全体の速度とは分けて扱います。

| 区間の中央値、ms | `bd` | `current` | `active` | `deferred` | `original_active` |
| --- | ---: | ---: | ---: | ---: | ---: |
| 入力 | 4.975 | 4.669 | 4.571 | 4.516 | 4.984 |
| 構築 | 1.532 | 2.146 | 1.923 | 1.879 | 1.592 |

出辺がある行の少ない場合に先読みを省くと構築の費用を減らせます。入次数判定を辺の入力中に保つ `original_active` は、追加の辺走査を避け、基準に最も小さな変更を加えます。この予備比較をもとに最終測定の候補としました。dense の小さな悪化や試行の変動もあるため、全ケースの正式測定は別に行います。

ヘッダーは重複保存せず、`bd29f70` と [CSR の差分](patches)で復元します。[driver.cpp](driver.cpp)は区間測定用です。[run.py](run.py)は凍結した実行内容を再構成し、共通ヘッダーは同一の `bd29f70` に固定します。CSR 以外の依存先がその版から変わっていないこと、5 つのパッチを再適用した結果が凍結ファイルと一致することを確認しました。パッチ適用には `patch` コマンドを使います。

```sh
python3 test/benchmark/results/shortest_path_round4_variants/active/run.py
```

出力先は ignored cache `.verify-helper/cache/shortest-path/round4/active/` です。公式問題データは `~/.cache/online-judge-tools/library-checker-problems/graph/shortest_path`、合成データは `.verify-helper/cache/shortest-path/round3/inputs/` に置きます。合成データは既存の `test/benchmark/shortest_path.py` の seed `20261019` のものです。保存時に絶対パスを相対パスへ置き換え、ヘッダーの原版と差分を固定しています。実行時と保存時の script hash を `manifest.json` に記録しました。
