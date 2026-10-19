# Shortest Path 追加測定

`e59415c` を基準に、入出力とグラフ構築を含めて比較します。前回の結果は [shortest_path.md](shortest_path.md) にあります。

```sh
python3 test/benchmark/shortest_path_round2.py \
  --output test/benchmark/results/shortest_path_round2_native \
  --trials 7 --native --topological --submissions \
  --reference fastest_407097=.verify-helper/cache/shortest-path/reference/407097.cpp \
  --reference reference_404592=.verify-helper/cache/shortest-path/reference/404592.cpp
```

`--submissions` は基準版・変更後・変更後の `<true>` の提出コードを追加します。基準版は `git show e59415c:...` で取得し、推移的な依存先も同じコミットから凍結します。`--native` を外すと generic 設定になります。`--only` はケース名を指定します。`--variant name=directory` で別のヘッダーフォルダを比較できます。`--reference name=source.cpp` は提出コードの全体時間を追加し、`--reference-only` は提出コードだけを測定します。`--topological` は基準版と変更後の `<true>` を追加します。`--option name=flag` で指定した候補だけにコンパイルフラグを追加できます。

測定ドライバは提出コードと同じ `DirectedCSR<uint32_t>` と `int64_t` 距離を使います。基準版は `vector<Edge>` に読み込んで CSR を構築します。変更後は一括整数入力と producer コンストラクタを使い、入力中に出次数を集計します。

| 項目 | 内容 |
| --- | --- |
| `parse_ms` | 入出力オブジェクトの準備と辺の読み込み。変更後は内部辺配列の確保と出次数の集計も含む |
| `build_ms` | 入力後から CSR 構築の完了まで。変更後は prefix sum と隣接辺の配置 |
| `solve_ms` | `dijkstra_path` の距離計算と経路復元 |
| `output_ms` | 距離と経路の出力、flush |
| `whole_ms` | プロセス起動・入出力・構築・計算・破棄・終了を含む時間 |

入力中に処理を移しているため、入力と構築は `parse_ms + build_ms` でも比較します。各区間の中央値の合計と、合計区間の中央値は一致するとは限りません。提出コード自体の `whole_ms` も別に測り、区間計測の固定費を含まない全体時間を比較します。

ヘッダーと提出コードの推移的な include を実行前に凍結し、SHA-256 を `environment.json` に保存します。CPU affinity を固定し、`/tmp/cp-wm-opt.lock` を共有します。バイナリと入力・出力は Linux の `/tmp` に置き、各ケースで 1 回 warmup 後、順序を変えて別プロセスで測ります。全試行と中央値を `results.json` に保存します。

Library Checker の公式 29 ケースと、前回と同じ seed `20261019` の合成 7 ケースを使います。warmup の出力を公式 checker、合成ケースを辺と距離による経路検証に通します。全試行で基準版と最短距離を照合し、検証時間は測定に含めません。

参照提出は [407097](https://judge.yosupo.jp/submission/407097) と [404592](https://judge.yosupo.jp/submission/404592) です。取得した API 応答とソース hash は [shortest_path_round2_reference.json](results/shortest_path_round2_reference.json) にあります。両者は一括入力・CSR・単一終点の打ち切りを使います。407097 は 3 フィールドの AVX2 入力、入力中の次数集計、上書き用配列を使います。両者の専用分岐・同距離処理・ソース内の最適化指定も全体時間に含まれるため、ヘッダー単体の差とは区別します。参照ソース自身の `#pragma GCC optimize`・`target` はそのまま有効です。

変更した処理です。

- `read_unsigned_records<3>` で 3 整数ずつ読みます。FPS の一括入出力を共通ヘッダーへ移し、Linux x86_64 / GCC の AVX2 経路を利用します。対応しない環境では通常の読み込みを使います。
- CSR の producer コンストラクタで入力中に出次数を数え、辺の再走査を省きます。内部の辺配列は上書き用に確保し、隣接辺の逆向き配置で cursor 用の offset コピーを省きます。同じ始点の辺順は維持します。
- 経路の頂点を `uint32_t` として出力し、既存の 4 桁テーブルを使います。
- `dijkstra_path` は始点から出次数 1 の区間を直接たどり、閉路を Brent 法で検出します。終点まで一本道ならキュー・全頂点の距離配列・親配列を作りません。重み・距離が整数で距離が固定幅の場合は、分岐点から残りの Dijkstra を実行します。

`O3` とループ展開も比較しました。Linux x86_64 / GCC 15.2、Core i7-13700F、CPU 2、native 設定、warmup 1 回・各 5 回の中央値です。単一出次数区間の処理を導入した段階の同一ソースを 3 条件で凍結し、各条件で公式 checker を通しました。全試行とソース 8 ファイルを [shortest_path_round2_compiler](results/shortest_path_round2_compiler) に保存しています。

| 実提出 `whole_ms` | `-O2` | `-O3` | `-O3 -funroll-loops` |
| --- | ---: | ---: | ---: |
| `line_00` | 38.54 | 43.67 | 41.72 |
| `almost_line_00` | 23.18 | 24.04 | 23.62 |
| `grid_random_00` | 9.80 | 9.51 | 10.03 |
| `max_dense_random_00` | 6.30 | 6.51 | 6.28 |
| `spfa_killer_00` | 25.47 | 27.26 | 26.85 |
| `wrong_dijkstra_killer_00` | 20.69 | 19.65 | 18.13 |
| `wrong_dijkstra_killer_01` | 30.81 | 33.08 | 35.28 |

一部には有効ですが、多くのケースで遅くなるため、Dijkstra 全体への `O3`・ループ展開の追加は見送りました。この比較の後に、浮動小数点の重みと整数距離を混在させる場合の分岐を調整しています。以下のソースを指定して当時の比較を再実行できます。

```sh
source=test/benchmark/results/shortest_path_round2_compiler/sources/test/dijkstra_path.cpp
python3 test/benchmark/shortest_path_round2.py \
  --output /tmp/shortest-path-compiler \
  --reference-only --official-only --native --trials 5 \
  --only line_00 almost_line_00 grid_random_00 max_dense_random_00 \
    spfa_killer_00 wrong_dijkstra_killer_00 wrong_dijkstra_killer_01 \
  --reference o2="$source" --reference o3="$source" --reference unroll="$source" \
  --option o3=-O3 --option unroll=-O3 --option unroll=-funroll-loops
```

検証の記録です。[一括入出力と FPS 回帰](results/shortest_path_io_validation.json) は GCC / Clang の通常実行・ASan / UBSan 計 8 件、[CSR](results/shortest_path_round2_csr_validation.json) は GCC / Clang・sanitizer・辺数不一致の例外を確認しています。[Dijkstra と依存先](results/shortest_path_round2_validation.json) の単体テスト・公式ケース・Boost の比較と、[ドキュメント](results/shortest_path_round2_docs.log) の全 149 ライブラリ生成・変更 4 bundle のコンパイルも記録しています。

正式測定は WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2、CPU 2 固定、各 7 回の中央値です。共通フラグは `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero`、native では `-march=native` を追加します。別 round の時間同士ではなく、各 round 内の変更前後を比較します。両設定の全 36 ケースで距離と経路を確認しました。単位は ms です。

[native の全試行](results/shortest_path_round2_native/results.json)と[環境・ソース hash](results/shortest_path_round2_native/environment.json)です。`own_path` は変更後の実提出コード、`own_topo` は同じコードの `<true>` です。

| native の実提出 `whole_ms` | 変更前 | `own_path` | `own_topo` | 407097 | 404592 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `line_00` | 51.34 | 32.41 | 31.79 | 21.95 | 19.64 |
| `almost_line_00` | 31.46 | 23.22 | 24.53 | 25.09 | 25.74 |
| `grid_random_00` | 18.10 | 10.24 | 9.92 | 10.76 | 11.01 |
| `grid_swirl_00` | 24.02 | 16.47 | 17.37 | 15.54 | 14.31 |
| `max_dense_random_00` | 10.19 | 6.02 | 6.37 | 6.53 | 6.35 |
| `max_dense_zero_00` | 9.95 | 5.64 | 5.82 | 5.86 | 5.54 |
| `max_sparse_random_00` | 18.28 | 9.48 | 14.05 | 14.59 | 9.31 |
| `max_sparse_random_01` | 23.55 | 14.96 | 19.87 | 21.28 | 15.60 |
| `max_star_00` | 19.27 | 11.64 | 17.91 | 9.23 | 7.86 |
| `max_star_01` | 26.68 | 18.26 | 20.39 | 10.58 | 8.25 |
| `spfa_killer_00` | 33.27 | 23.33 | 28.74 | 23.71 | 22.62 |
| `wrong_dijkstra_killer_00` | 28.27 | 19.24 | 13.54 | 11.89 | 19.45 |
| `wrong_dijkstra_killer_01` | 40.04 | 29.65 | 28.07 | 30.27 | 29.74 |
| `synthetic_random` | 20.77 | 14.17 | 14.94 | 15.17 | 13.08 |
| `synthetic_zero` | 17.69 | 10.37 | 12.39 | 9.93 | 8.70 |
| `synthetic_binary` | 19.90 | 12.18 | 14.46 | 15.56 | 13.32 |
| `synthetic_relaxation` | 19.69 | 14.21 | 6.96 | 7.07 | 12.44 |
| `synthetic_disconnected` | 23.03 | 16.66 | 13.68 | 12.59 | 16.47 |

同じ native round の区間測定です。入力＋構築は各試行の `parse_ms + build_ms` の中央値です。計算には経路復元を含みます。提出コード全体の表とは、区間計測用の採時・JSON 出力の有無が異なります。

| 区間測定 | 入力＋構築・前 | 入力＋構築・後 | 計算・前 | 計算・後 | 出力・前 | 出力・後 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `line_00` | 16.60 | 10.11 | 27.09 | 15.92 | 6.86 | 4.98 |
| `almost_line_00` | 15.97 | 8.33 | 11.53 | 11.46 | 2.35 | 1.72 |
| `grid_random_00` | 13.51 | 9.08 | 0.87 | 0.84 | 0.02 | 0.02 |
| `max_dense_random_00` | 8.95 | 5.05 | 0.17 | 0.17 | 0.02 | 0.02 |
| `max_sparse_random_01` | 17.04 | 8.14 | 5.56 | 5.32 | 0.03 | 0.02 |
| `spfa_killer_00` | 16.85 | 9.41 | 12.20 | 11.66 | 1.88 | 1.39 |
| `wrong_dijkstra_killer_00` | 14.95 | 6.58 | 11.33 | 11.26 | 0.03 | 0.02 |
| `wrong_dijkstra_killer_01` | 17.11 | 8.73 | 17.02 | 16.58 | 3.37 | 2.46 |
| `synthetic_random` | 14.70 | 9.26 | 4.20 | 4.13 | 0.03 | 0.02 |
| `synthetic_zero` | 13.85 | 7.10 | 2.04 | 1.84 | 0.48 | 0.35 |
| `synthetic_relaxation` | 9.54 | 5.25 | 8.82 | 8.07 | 0.04 | 0.04 |

[generic の全試行](results/shortest_path_round2_generic/results.json)と[環境・ソース hash](results/shortest_path_round2_generic/environment.json)です。参照提出の追加測定は native round で行いました。

| generic の実提出 `whole_ms` | 変更前 | `own_path` | `own_topo` |
| --- | ---: | ---: | ---: |
| `line_00` | 53.24 | 31.82 | 31.47 |
| `almost_line_00` | 30.93 | 21.95 | 23.92 |
| `grid_random_00` | 15.05 | 9.16 | 9.56 |
| `grid_swirl_00` | 23.92 | 16.30 | 18.09 |
| `max_dense_random_00` | 10.99 | 6.40 | 7.11 |
| `max_dense_zero_00` | 9.87 | 6.01 | 6.02 |
| `max_sparse_random_00` | 18.86 | 12.53 | 15.45 |
| `max_sparse_random_01` | 22.25 | 14.46 | 18.28 |
| `max_star_00` | 19.32 | 12.21 | 17.26 |
| `max_star_01` | 25.11 | 17.80 | 20.04 |
| `spfa_killer_00` | 30.54 | 21.07 | 25.30 |
| `wrong_dijkstra_killer_00` | 26.60 | 19.28 | 14.00 |
| `wrong_dijkstra_killer_01` | 38.81 | 27.36 | 28.42 |
| `synthetic_random` | 18.78 | 12.40 | 14.25 |
| `synthetic_zero` | 17.06 | 9.78 | 11.95 |
| `synthetic_binary` | 18.73 | 11.53 | 13.93 |
| `synthetic_relaxation` | 18.56 | 13.25 | 6.83 |
| `synthetic_disconnected` | 23.55 | 16.45 | 13.27 |

`almost_line`・`grid`・密グラフなどで、入出力と構築を含む差が縮まりました。`line_00` には差が残ります。参照提出は全出次数または全入次数が 1 以下の場合に CSR を作らず専用の隣接配列を使い、入力・構築も含む処理全体が異なります。`<true>` は固定費があり、常に既定版より速いわけではありません。
