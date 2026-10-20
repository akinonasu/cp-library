# 疎グラフの速度変動の診断

`max_sparse_random_01` で報告した約 9.7% の悪化を、前版 `f4aa743` と前回変更後 `bd29f70` から再現しました。15 回・41 回の追加比較では固定的な悪化を再現できませんでした。構築の変動位置は、CSR の `arcs_[--offset_[from]]` による辺配置です。cache、CPU の動作、WSL ホストなど、その下の原因は未確定です。

このディレクトリは原因調査の記録です。最終実装の正式な速度比較とは区別してください。

## 条件

WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2、CPU affinity 2 です。[環境](system.json)に詳細を保存しました。共通フラグは `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=native` です。

共有 lock `/tmp/cp-wm-opt.lock` の下でコンパイル・測定し、入力・出力・バイナリは Linux `/tmp` に置きました。各ケース 1 回の warmup 後、試行ごとに実行順をランダムに変え、毎回新しいプロセスで測りました。warmup の各出力は公式 checker、全試行の距離は相互照合で検証しています。checker の時間は測定から除いています。

`own_f4`・`own_bd` は各版の実提出コードです。ヘッダーも推移的な依存先を同じ Git revision から取得しました。`phase_f4`・`phase_bd` は `bd29f70:test/benchmark/shortest_path_round3.cpp` に各版のヘッダーを組み合わせ、前版では `SP_BASELINE` を定義しています。`phase_` は採時・JSON 出力があるため、実提出の全体時間とは分けて扱います。

## 再現

[15 回の全試行](runs/exact/results.json)と[条件・原版 hash](runs/exact/environment.json)です。seed は `2026101904` です。表は実提出の全体時間の中央値、単位は ms です。

| ケース | `f4aa743` | `bd29f70` |
| --- | ---: | ---: |
| `max_sparse_random_01` | 14.076 | 13.840 |
| `line_00` | 30.997 | 20.256 |
| `max_star_01` | 17.498 | 10.256 |
| `wrong_dijkstra_killer_01` | 29.244 | 26.780 |
| `grid_swirl_00` | 17.040 | 15.954 |
| `max_sparse_random_00` | 9.737 | 9.219 |

疎グラフの分布には遅い試行が残ります。

| `max_sparse_random_01` 実提出 | 最小 | 25% 点 | 中央値 | 75% 点 | 最大 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `f4aa743` | 13.517 | 13.858 | 14.076 | 15.233 | 17.187 |
| `bd29f70` | 13.291 | 13.585 | 13.840 | 14.375 | 17.196 |

この run の区間測定で、構築は前後とも約 3–6 ms に変動しました。探索の中央値は前版 5.174 ms、変更後 4.958 ms でした。前回の 7 回測定でも構築の変動があり、遅い試行が中央値の両側にどう分布したかによって実提出と区間ドライバの結論が反転しています。

## メモリ割り当ての診断

[41 回の全試行](runs/fault/results.json)と[条件](runs/fault/environment.json)です。seed は `2026101905` です。実提出には外側から `getrusage` の minor/major fault、context switch、user/system 時間を記録しました。診断用 `phase_` は追加で CSR 配列のアドレスと構築中の fault 数を記録します。

両版の診断用区間ドライバで、構築の minor fault は全 82 試行で 977、major fault は 0 でした。Arc 配列のアドレスの下位 21 bit は全試行で `0x8d010`、offset・入力辺配列との相対位置も一定でした。ASLR によって上位アドレスは変化しています。THP は `madvise` 設定です。

同 run の実提出の中央値は前版 14.596 ms、変更後 13.965 ms でした。遅い試行を fault 数の増加や、このアドレスの下位 bit の違いでは説明できません。

## 構築区間の分解

[41 回の全試行](runs/finish/results.json)と[条件](runs/finish/environment.json)です。seed は `2026101906` です。CSR の `finish` を次の区間に分けました。

| 区間 | 内容 | 観測した時間の目安 |
| --- | --- | ---: |
| `prefix_ms` | 次数の累積和 | 0.05–0.06 ms |
| `alloc_ms` | Arc 配列の確保・初期化 | 0.8 ms |
| `scatter_ms` | 入力順から CSR への辺配置 | 約 2 ms または約 5 ms |

Arc 配列の確保・初期化の minor fault は 977、辺配置は 0 でした。構築の約 3 ms の差は辺配置の区間で発生しています。次数・辺の型や辺順は前後で同じです。ランダムな offset・Arc の参照先を待つ箇所が改善候補です。より深い原因が cache や CPU の動作かどうかは、この測定だけでは確定しません。

診断には `getrusage`・時刻取得・アドレス保存・ループ最終回の判定を加えています。生成コードにも影響するため、診断区間の絶対時間を通常版の時間として使いません。

## 先読み候補の診断

[41 回の全試行](runs/prefetch/results.json)と[条件](runs/prefetch/environment.json)です。seed は `2026101907` です。前回変更後 `bd29f70` と、32 辺先の offset・Arc を `__builtin_prefetch` で先読みする候補を、同じ run で比較しました。[候補の差分](patches/prefetch_own.patch)を保存しています。この候補は正式実装を選ぶ前の比較です。

| 指標の中央値 | `bd29f70` | 先読み候補 |
| --- | ---: | ---: |
| 実提出の全体、ms | 13.616 | 12.773 |
| 診断用ドライバの辺配置、ms | 2.050 | 1.195 |
| 診断用ドライバの確保・初期化、ms | 0.812 | 0.801 |
| 確保・初期化の minor fault | 977 | 977 |
| 辺配置の minor fault | 0 | 0 |

辺配置の最小–最大は、基準 1.976–5.085 ms、候補 1.140–1.454 ms でした。待ち時間の短縮を観測しましたが、41 回の範囲外で変動がなくなる保証ではありません。

## 保存物と再実行

[summary.json](summary.json)は分位点と区間の集計、各 `runs/*/results.json` は全試行と実行順です。[source_manifest.json](source_manifest.json)は各実行に使った凍結ファイルの hash、[patches](patches)と[drivers](drivers)は診断変更です。元のヘッダー一式は Git revision から再取得するため複製していません。`environment.json` の `sources` の hash は診断前の原版、`source_manifest.json` の hash は診断変更後の保存ファイルです。

実行時のスクリプトから、絶対パスを相対パスに置き換え、同じ内容の driver と凍結処理を固定して保存しました。[script_provenance.json](script_provenance.json)に実行時と保存時の hash を記録しています。分位点は、試行をソートした配列の添字 `n/4`・`3n/4` を整数に切り捨てて選び、中央値は中央 1 要素です。

リポジトリのルートで順に実行します。[freeze_sources.py](freeze_sources.py)に保存した凍結処理を利用します。

```sh
python3 test/benchmark/results/shortest_path_round4_diagnosis/reproduce.py
python3 test/benchmark/results/shortest_path_round4_diagnosis/fault_probe.py
python3 test/benchmark/results/shortest_path_round4_diagnosis/finish_probe.py
python3 test/benchmark/results/shortest_path_round4_diagnosis/prefetch_probe.py
python3 test/benchmark/results/shortest_path_round4_diagnosis/validate.py
```

出力先は ignored cache `.verify-helper/cache/shortest-path/round4/reproduce/` です。公式問題データは `~/.cache/online-judge-tools/library-checker-problems/graph/shortest_path` に置きます。WSL の PATH は GCC 15.2 が選ばれるように設定してください。CPU affinity 2・共有 lock・native フラグはスクリプト内で固定しています。

[検証結果](validation.json)では原版 130 hash、凍結ファイル 138 hash、パッチ 7 本、driver 6 本、全試行数を照合しました。`validate.py` は ignored cache の凍結ファイルと保存資料の整合性を確認します。
