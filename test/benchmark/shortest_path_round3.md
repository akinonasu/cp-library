# Shortest Path 追加測定 3

`f4aa743` を基準に、一本道・星型など、前回差が残ったケースを比較します。[前回の測定](shortest_path_round2.md)も参照してください。

```sh
python3 test/benchmark/shortest_path_round3.py \
  --output test/benchmark/results/shortest_path_round3_native \
  --trials 7 --native --topological --submissions \
  --reference fastest_407097=.verify-helper/cache/shortest-path/reference/407097.cpp \
  --reference reference_404592=.verify-helper/cache/shortest-path/reference/404592.cpp
```

`--native` を外すと generic 設定です。`--submissions` は変更前・変更後・変更後の `<true>` の実提出コードを加えます。`--topological` は区間測定にも前後の `<true>` を加えます。`--only`・`--variant name=directory`・`--option name=flag` で候補を比較できます。

変更前のヘッダーと実提出コードは `git show f4aa743:...` から取得し、推移的な依存先も同じ版に揃えます。自作 variant の凍結ソースを結果の `sources/` に保存し、SHA-256 を `environment.json` に記録します。提出コードの include は保存先内の相対パスに変換します。保存時の `.cpp` は judge テストとして検出されない名前にします。第三者の参照ソースは ignored cache だけに置き、取得時の hash を記録します。

| 区間 | 内容 |
| --- | --- |
| `parse_ms` | 入出力の準備・整数入力・辺配列の確保・次数集計 |
| `build_ms` | 入力後からグラフ構築完了まで |
| `solve_ms` | `dijkstra_path` の距離計算・経路復元 |
| `output_ms` | 距離と経路の出力・flush |
| `whole_ms` | プロセス起動・入出力・構築・計算・破棄・終了 |

入力と構築は `parse_ms + build_ms` でも比較します。実提出コードの全体時間は別に測り、区間測定の採時と JSON 出力を含むドライバと区別します。

公式 29 ケースと seed `20261019` の合成 7 ケースを使います。CPU affinity を固定し、共有 lock `/tmp/cp-wm-opt.lock` の下で測ります。バイナリと入力・出力は Linux の `/tmp` に置き、各ケース 1 回 warmup 後、順序を変えて別プロセスで測定します。全試行の距離を照合し、warmup の経路を公式 checker または辺と距離による検証に通します。検証時間は測定から除きます。

参照は [407097](https://judge.yosupo.jp/submission/407097) と [404592](https://judge.yosupo.jp/submission/404592) です。現在の順位・API 応答・ソース hash を [shortest_path_round3_reference.json](results/shortest_path_round3_reference.json) に保存しています。両者は全出次数または全入次数が 1 以下の場合に、前向きまたは逆向きの一意な経路をたどります。一般のグラフは Dijkstra です。入力形式・同距離処理・専用配列・ソース内の最適化指定も異なるため、全体時間をヘッダー単体の性能とはみなしません。参照自身の `#pragma GCC optimize`・`target` はそのまま有効です。

公式ケースの構造です。重みや頂点番号による入力認識は行いません。

| ケース | 頂点数 | 辺数 | 構造 |
| --- | ---: | ---: | --- |
| `line_00` | 500000 | 499999 | 全出次数・全入次数とも 1 以下 |
| `max_star_00` | 500000 | 499999 | 全入次数 1 以下。sink 499968 頂点 |
| `max_star_01` | 500000 | 499999 | 全入次数 1 以下。sink 250000 頂点 |

CSR の配置方法を比較した予備測定を [shortest_path_round3_variants/layout](results/shortest_path_round3_variants/layout) に保存しています。native 設定、CPU 2、warmup 1 回・各 5 回で、自作ソース 6 種類と全試行を残しています。`reserve(n)` と追加の Arc cache も比較し、安定した改善が得られないため見送りました。

| `line_00` の実提出全体 | 変更前 | 直接参照 | 直接参照＋直接配置 | 直接参照＋直接配置＋`reserve(n)` |
| --- | ---: | ---: | ---: | ---: |
| 予備測定、ms | 32.80 | 24.17 | 22.04 | 22.52 |

予備測定同士でソース・試行が異なるため、正式測定の表とは直接比較しません。ソース内の頂点・重み・距離の型、通常の `g[v]` の辺順と連続性は維持します。

探索の比較は [search-round3](results/shortest_path_round3_variants/search-round3)、[search-round3b](results/shortest_path_round3_variants/search-round3b)、[search-unique-round3](results/shortest_path_round3_variants/search-unique-round3)、[search-unique-round3b](results/shortest_path_round3_variants/search-unique-round3b) に保存しています。いずれも native 設定、CPU 2、warmup 1 回・各 5 回です。以下は計算・経路復元区間の中央値です。

| 比較 | ケース | 基準、ms | 候補、ms |
| --- | --- | ---: | ---: |
| radix bucket が 1 要素なら直接取り出す | `grid_swirl_00` | 7.05 | 6.34 |
| 入次数 1 以下のグラフで一本道を直接たどる | `max_star_01` | 8.71 | 0.52 |

sink の距離を 0 で初期化する方法、sink の早期分岐、終点の距離から許容重みを計算する方法も比較しました。他のケースの悪化があり、採用を見送りました。入次数の判定は頂点ごとの bit で重複を検出し、一意な経路が保証される場合に使います。追加の逆辺配列は持ちません。

出力の予備比較は [output](results/shortest_path_round3_variants/output) に保存しています。native 設定、CPU 2、warmup 1 回・各 7 回です。絶対 include を保存先内の相対 include に変更し、変更前後の hash と対応先を manifest に残しています。予備版は一時文字バッファを経由し、正式版は `FastOutput` の内部バッファへ直接書くため、同じ実装として扱いません。

| `line_00` の出力・flush 区間 | 1 整数ずつ出力、ms | まとめて出力、ms |
| --- | ---: | ---: |
| inline の予備版 | 4.89 | 3.03 |
| noinline の予備版 | 4.93 | 3.24 |

予備 run の全体時間には大きなばらつきがあり、出力区間の改善だけを確認しました。正式版の全体時間は以下の 7 回測定で比較します。

正式測定は WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2、CPU 2 固定、各 7 回の中央値です。共通フラグは `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero`、native は `-march=native` を追加します。以下は同じ run 内の比較で、単位は ms です。

[native の全試行](results/shortest_path_round3_native/results.json)と[環境・凍結ソース](results/shortest_path_round3_native/environment.json)です。`<true>` は変更後の `dijkstra_path<true>` です。参照提出は native で比較しました。

| 実提出 `whole_ms` | 変更前 | 変更後 | `<true>` | 407097 | 404592 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `line_00` | 31.74 | 20.71 | 21.14 | 21.85 | 18.08 |
| `almost_line_00` | 24.16 | 21.43 | 23.40 | 24.40 | 24.42 |
| `grid_random_00` | 11.73 | 8.73 | 10.48 | 11.31 | 10.45 |
| `grid_swirl_00` | 16.54 | 15.33 | 15.93 | 15.59 | 14.01 |
| `max_dense_random_00` | 6.01 | 6.03 | 6.19 | 6.81 | 6.15 |
| `max_dense_zero_00` | 5.86 | 5.97 | 6.02 | 6.12 | 6.04 |
| `max_sparse_random_00` | 9.34 | 8.99 | 8.66 | 14.10 | 9.11 |
| `max_sparse_random_01` | 15.39 | 16.88 | 18.64 | 21.66 | 17.07 |
| `max_star_00` | 10.86 | 9.04 | 8.83 | 8.54 | 7.44 |
| `max_star_01` | 16.75 | 9.90 | 9.91 | 9.52 | 8.84 |
| `spfa_killer_00` | 21.55 | 21.36 | 25.36 | 22.26 | 21.21 |
| `wrong_dijkstra_killer_00` | 20.07 | 19.14 | 12.49 | 12.01 | 20.36 |
| `wrong_dijkstra_killer_01` | 29.47 | 27.41 | 28.81 | 27.36 | 28.57 |
| `synthetic_random` | 12.18 | 12.47 | 14.53 | 15.28 | 14.79 |
| `synthetic_zero` | 10.93 | 10.00 | 12.04 | 11.82 | 8.69 |
| `synthetic_binary` | 11.74 | 11.81 | 14.31 | 14.63 | 12.88 |
| `synthetic_relaxation` | 14.00 | 13.83 | 6.94 | 6.54 | 12.39 |
| `synthetic_disconnected` | 16.56 | 16.46 | 13.44 | 12.64 | 16.36 |

同じ native run の区間測定です。入力＋構築は、各試行の `parse_ms + build_ms` の中央値です。計算には経路復元を含みます。実提出コードとは採時・JSON 出力の有無が異なるため、全体時間は上の表で比較します。

| 区間 | 入力＋構築・前 | 入力＋構築・後 | 計算・前 | 計算・後 | 出力・前 | 出力・後 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `line_00` | 9.50 | 9.05 | 15.79 | 7.50 | 4.98 | 2.99 |
| `almost_line_00` | 8.22 | 8.31 | 11.01 | 10.87 | 1.71 | 0.92 |
| `grid_random_00` | 8.67 | 9.26 | 0.83 | 0.78 | 0.02 | 0.02 |
| `grid_swirl_00` | 7.21 | 7.39 | 6.87 | 6.28 | 0.96 | 0.67 |
| `max_dense_random_00` | 4.97 | 5.09 | 0.17 | 0.16 | 0.02 | 0.02 |
| `max_dense_zero_00` | 5.09 | 4.99 | 0.01 | 0.01 | 0.01 | 0.01 |
| `max_sparse_random_00` | 7.86 | 7.78 | 0.56 | 0.00 | 0.02 | 0.02 |
| `max_sparse_random_01` | 10.66 | 7.72 | 5.26 | 4.97 | 0.02 | 0.02 |
| `max_star_00` | 6.30 | 6.36 | 3.49 | 1.35 | 0.02 | 0.02 |
| `max_star_01` | 7.18 | 7.97 | 8.03 | 0.56 | 0.03 | 0.02 |
| `spfa_killer_00` | 7.84 | 8.13 | 10.81 | 10.55 | 1.39 | 0.77 |
| `wrong_dijkstra_killer_00` | 7.22 | 6.86 | 11.95 | 11.07 | 0.03 | 0.03 |
| `wrong_dijkstra_killer_01` | 8.45 | 8.68 | 16.99 | 15.72 | 2.52 | 1.50 |
| `synthetic_random` | 7.04 | 6.71 | 4.08 | 4.07 | 0.02 | 0.02 |
| `synthetic_zero` | 7.05 | 7.40 | 1.78 | 1.85 | 0.35 | 0.21 |
| `synthetic_binary` | 7.13 | 7.14 | 3.85 | 3.88 | 0.13 | 0.08 |
| `synthetic_relaxation` | 4.99 | 5.07 | 7.68 | 7.84 | 0.03 | 0.03 |
| `synthetic_disconnected` | 7.95 | 7.81 | 7.30 | 6.88 | 0.03 | 0.03 |

[generic の全試行](results/shortest_path_round3_generic/results.json)と[環境・凍結ソース](results/shortest_path_round3_generic/environment.json)です。

| 実提出 `whole_ms` | 変更前 | 変更後 | `<true>` |
| --- | ---: | ---: | ---: |
| `line_00` | 31.59 | 21.55 | 20.49 |
| `almost_line_00` | 22.82 | 21.75 | 24.72 |
| `grid_random_00` | 10.12 | 9.40 | 9.89 |
| `grid_swirl_00` | 15.74 | 15.54 | 16.40 |
| `max_dense_random_00` | 6.75 | 6.90 | 6.88 |
| `max_dense_zero_00` | 6.41 | 6.21 | 6.19 |
| `max_sparse_random_00` | 9.21 | 8.74 | 8.88 |
| `max_sparse_random_01` | 14.02 | 13.85 | 17.90 |
| `max_star_00` | 11.48 | 9.75 | 9.55 |
| `max_star_01` | 16.82 | 9.83 | 10.24 |
| `spfa_killer_00` | 21.43 | 20.39 | 25.05 |
| `wrong_dijkstra_killer_00` | 20.11 | 19.08 | 14.37 |
| `wrong_dijkstra_killer_01` | 28.22 | 25.55 | 28.21 |
| `synthetic_random` | 13.03 | 12.56 | 14.30 |
| `synthetic_zero` | 12.12 | 11.90 | 13.97 |
| `synthetic_binary` | 11.43 | 11.52 | 14.57 |
| `synthetic_relaxation` | 13.78 | 13.51 | 6.80 |
| `synthetic_disconnected` | 15.74 | 15.90 | 13.33 |

`line_00` の native 全体時間は 31.74 → 20.71 ms、`max_star_01` は 16.75 → 9.90 ms でした。一本道の直接参照・一意な経路のキュー省略・一括出力が寄与しています。参照 404592 の `line_00` 18.08 ms には差が残ります。

`max_sparse_random_01` の native 実提出は 15.39 → 16.88 ms と約 9.7% 遅くなりました。generic は 14.02 → 13.85 ms でした。全ケースが改善するわけではありません。`<true>` も全体の次数走査に固定費があり、既定版より常に速いわけではありません。両設定の全 36 ケースで距離・経路を確認しました。

検証の記録は [CSR](results/shortest_path_round3_csr_validation.json)、[一括出力・一括入力・FPS 入出力](results/shortest_path_round3_io_validation.json)、[Dijkstra と依存先](results/shortest_path_round3_validation.json)、[ドキュメント](results/shortest_path_round3_docs.log) に保存しています。CSR は GCC・Clang の通常実行と ASan / UBSan の 4 件、入出力は同じ 4 条件を 3 つの unit に適用した 12 件です。ドキュメントは全 150 ライブラリ・125 judge の生成と、変更 6 bundle のコンパイルを確認しました。

Dijkstra 関連は GCC・Clang の通常 unit 14 件、GCC の ASan / UBSan 6 件、公式 29 ケース × 3 提出 × 2 コンパイラの 174 件、Boost 多倍長整数を使う 2 サンプル・60 個の Fraction oracle ケースを各コンパイラで確認し、すべて成功しました。
