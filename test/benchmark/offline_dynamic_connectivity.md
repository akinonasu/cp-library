# Offline Dynamic Connectivity

2026-10-20。`graph/connectivity/offline_dynamic_connectivity.hpp` を追加した。既存の Rollback Union Find を変更せずに利用する。時間区間の適用を `components/offline_time_segments.hpp`、符号なし64 bitキーの安定ソートを `components/radix_sort.hpp` に分離した。標準 C++ のみを使い、x86 専用命令は使わない。

## API

辺の追加・削除、連結判定・成分サイズ・成分数の問い合わせを登録して `solve()` で一括計算する。並行辺は本数が0と正の間で変化する有効区間にまとめる。自己ループは無視し、存在しない辺の削除は何もしない。同じ辺の端点の順序は区別しない。

任意の問い合わせには `query()`・`run(visit)` を使う。併合とその取り消しを通知する `run(on_link,on_cut,visit)` では、外部に頂点の成分和などを管理できる。問い合わせは必ず時刻順で、全ての成功した併合は最後に取り消す。頂点加算を含む成分和のドライバーでは、外部にも Union Find の親を記録し、頂点への加算を祖先へ伝播する。

## 参考提出

Library Checker の [Dynamic Graph Vertex Add Component Sum](https://judge.yosupo.jp/problem/dynamic_graph_vertex_add_component_sum) の AC 上位20件を取得し、C++ 上位3件を確認した。一覧・取得時刻・各ソースの SHA-256 は [references.json](results/offline_dynamic_connectivity/references.json)。

| 提出 | 取得時の最大時間 | 確認した方式 |
| --- | ---: | --- |
| [402761](https://judge.yosupo.jp/submission/402761) | 57 ms | 削除時刻を利用する expiry AM forest。森林の回転と部分木集計、独自の入力処理 |
| [404999](https://judge.yosupo.jp/submission/404999) | 58 ms | 同じ expiry AM forest の方式 |
| [340473](https://judge.yosupo.jp/submission/340473) | 64 ms | 削除時刻を先読みし、集計を持つ AM Tree の辺重みとして利用 |

いずれも Rollback Union Find と時間セグメント木の組み合わせではなかった。今回は依頼された Rollback Union Find を利用する方式を実装し、上位提出の別方式へは置き換えていない。最速提出より常に速いという結果ではない。

## 高速化

- 辺の有効期間を、問い合わせを登録した時点だけに圧縮する。辺更新だけが続く部分に葉を作らず、問い合わせを含まない有効区間も捨てる。
- ハッシュ表を使わず、正規化した辺キーを安定基数ソートする。同じ辺の操作順を保つため、同じ時刻での重複追加や存在しない辺の削除も扱える。
- 8 bitずつの安定な分配を使い、共通のバイトと既に整列した入力を省く。小さい入力には安定な比較ソートを使う。
- 時間セグメント木の全バケットを CSR と同じ形の1本の配列へ格納する。要素数の累積和を格納時に書き換え、書き込み位置のコピーを作らない。
- 既に同じ成分にある辺は Union Find の履歴に記録しない。成功した併合は同時に高々 `n-1` 回なので、履歴の予約もそこまでに制限する。
- 有効区間がない場合は時間セグメント木の配列を確保せず、問い合わせを順に呼び出す。

入力のファイル名・乱数 seed・問題固有の頂点番号には依存しない。DFS は時間セグメント木だけを再帰し、深さは対数。入力グラフの長いパスで再帰が深くなることはない。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2。`-std=c++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero -DONLINE_JUDGE`。ウォームアップ1回、7回測定し、各ラウンドの実行順を固定 seed でシャッフル。他のコンパイルやテストとは同時に測定しない。

公式生成器の commit は `1814c4e5205517e368bb57a8d1127eb961cfeaae`。入力・対象ソースのハッシュ、CPU・コンパイラ、全試行は [measurements.json](results/offline_dynamic_connectivity/measurements.json)。Mac の速度は測っていない。オンライン提出はしていない。

## ライブラリの比較

通常の `vector<vector<int>>` の時間セグメント木と、辺更新の各時点にも葉を作る版を比較した。前者は適用順も同じになるように区間を逆順に格納し、CSR 版との違いをバケットの持ち方に揃えた。後者は辺更新後にも空の問い合わせ位置を挿入する。どの版も同じ基数ソートと Rollback Union Find、成分和のコールバックを使う。

入力後から、登録・前処理・時間セグメント木の構築・連結性処理・成分和処理・チェックサムを計測する。入力・出力と最終的なグラフオブジェクトの破棄は含めない。`run` 内部で確保・解放する作業領域は含む。全版の答えのチェックサムと個数を照合した。次は中央値、ms。

| 入力 | 今回 | 各節点に vector | 全操作に時刻を割当 |
| --- | ---: | ---: | ---: |
| link_and_cut_00 | 60.44 | 57.44 | 99.98 |
| max_random_00 | 23.13 | 39.00 | 28.95 |
| dense_00 | 30.54 | 42.55 | 38.41 |
| uv_swapped_00 | 21.29 | 36.35 | 27.02 |
| random_00 | 21.59 | 35.51 | 27.30 |
| 問い合わせが2回だけの長いパス | 8.84 | 8.95 | 33.32 |
| 追加・問い合わせ・削除の反復 | 5.22 | 8.46 | 8.30 |
| 並行辺をまとめて追加・削除 | 0.43 | 0.44 | 1.11 |
| 問い合わせのみ | 1.10 | 1.13 | 1.12 |

CSR は主要なランダム入力で約28～41%短縮した一方、`link_and_cut_00` は各節点に vector を持つ版の方が約5%速かった。全時刻を使う版に対しては、辺更新が多く問い合わせの少ない場合に大きな差が出た。常に一定倍率で速いとは扱わない。

ピーク RSS はプロセス全体の値で、入力の保存領域も含む。`max_random_00` は今回約37.4 MiB、各節点に vector を持つ版が約49.2 MiB。`link_and_cut_00` は約42.2 MiBと約43.0 MiB。分布によってメモリの差も異なる。

## ソートの比較

初期の比較ソート版の7回結果は [measurements_comparison_sort.json](results/offline_dynamic_connectivity/measurements_comparison_sort.json)。それとは別に、比較ソートと8・11・16 bitの安定基数ソートを同じラウンドで3回比較し、[sort_screen.json](results/offline_dynamic_connectivity/sort_screen.json) に保存した。

この比較で8 bitの候補は、ライブラリ部分の `link_and_cut_00` が約74.19 msから62.38 ms、`max_random_00` が29.05 msから23.23 ms、`dense_00` が40.18 msから32.52 msとなった。16 bitの候補が短い分布もあったが、256個のカウンタで扱える8 bitを採用した。最終版では、既に整列した場合の省略も加えている。別時点の測定結果同士を厳密な同時比較とは扱わない。

## 最速提出との比較

次はプロセス起動・入力・登録・全処理・出力・終了を含む中央値、ms。通常ファイルから入力し、出力は `/dev/null`。今回のドライバーは FastInput/FastOutput、参考提出は元の専用入出力を使う。アルゴリズム単体の比較ではない。

| 入力 | 今回の提出コード | 提出402761 |
| --- | ---: | ---: |
| link_and_cut_00 | 71.18 | 37.85 |
| max_random_00 | 37.00 | 17.84 |
| dense_00 | 40.40 | 17.35 |
| uv_swapped_00 | 35.51 | 17.59 |
| random_00 | 34.45 | 15.86 |

この比較では、削除時刻を使う別方式の最速提出の方が速かった。公開ジャッジの57 msと上表のローカル時間は直接比較しない。

## 検証

- 時間区間部品は時刻数0～70で、空区間・重複・非2冪の境界を含む200区間を素朴な配列と比較。開始時の状態へ戻ることと、繰り返し実行も確認。
- 基数ソートは0・1・255・256・257・10000要素などで安定な比較ソートと照合。64 bit全域・重複・上位バイトのみの差・整列済み・逆順・`uint32_t`・`vector<bool>`・コピー不可の要素を確認。
- 動的連結性は250通り・各400操作を BFS と比較。連結性・成分サイズ・成分数・問い合わせ ID・コピー・reset・再計算を確認。
- 成分和は150通り・各300操作を BFS と比較。負の値、並行辺、頂点加算、併合と取り消しの対応、最後の外部データも確認。
- 空の状態、自己ループ、存在しない辺の削除、端点を入れ替えた削除、問い合わせのない状態、10万頂点の長いパス。
- GCC 15.2.0・Clang 21.1.8、Clang ASan/UBSan。公式全19ケースを両コンパイラで確認。
- 参考提出402761は初期比較で全19ケースを確認し、変更していないため最終版では再テストを省いた。
- 説明内コードの実行、GitHub Pages の生成、ヘッダーと提出コードの Bundle。

対象は [selection.json](results/offline_dynamic_connectivity/selection.json)。基数ソート追加後は変更に依存する部分だけを `selection_radix.json` で再検証し、時間区間部品の成功済みテストは繰り返していない。各ログは同じディレクトリに保存する。

```sh
python3 test/benchmark/offline_dynamic_connectivity.py --references
flock /tmp/cp-wm-opt.lock python3 test/benchmark/offline_dynamic_connectivity.py
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/offline_dynamic_connectivity/selection.json
python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/offline_dynamic_connectivity/selection.json
```
