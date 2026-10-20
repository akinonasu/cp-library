# Shortest Path

Linux x86_64 の GCC を使い、距離計算と入出力・グラフ構築・経路復元を分けて測定します。基準版は `91a7388`、時間の単位は ms です。

```sh
python3 test/benchmark/shortest_path_reference.py --ids 407097 404592
python3 test/benchmark/shortest_path.py \
  --output test/benchmark/results/shortest_path_generic \
  --csr-variant csr=. --csr-target-variant csr_path=. \
  --csr-topological-target-variant csr_topo_path=. --trials 7 --no-heap \
  --reference own_path=test/dijkstra_path.test.cpp \
  --reference own_topo=test/dijkstra_topological.test.cpp
python3 test/benchmark/shortest_path.py \
  --output test/benchmark/results/shortest_path_native \
  --csr-variant csr=. --csr-target-variant csr_path=. \
  --csr-topological-target-variant csr_topo_path=. --trials 7 --no-heap --native \
  --reference own_path=test/dijkstra_path.test.cpp \
  --reference own_topo=test/dijkstra_topological.test.cpp \
  --reference fastest_407097=.verify-helper/cache/shortest-path/reference/407097.cpp \
  --reference reference_404592=.verify-helper/cache/shortest-path/reference/404592.cpp
```

`--problem` は Library Checker の `graph/shortest_path` フォルダです。既定値は online-judge-tools のキャッシュです。公式入力の `in/*.in` と、生成する 7 種類の入力を使います。`--only` でケース名を指定できます。`--variant name=directory` は `directory/graph/dijkstra.hpp` と推移的な依存先を複製して比較します。`--target-variant` は同じフォルダの `dijkstra_path` を使います。`--topological-variant` と `--topological-target-variant` は `<true>` を指定します。`--csr-variant` と `--csr-target-variant` は重み `uint32_t`、距離 `int64_t` の `DirectedCSR` を使います。`--csr-topological-variant` と `--csr-topological-target-variant` は両方を組み合わせます。`--reference name=source.cpp` は提出コードなどの全体時間を追加します。参照だけを測る場合は `--reference-only` を使います。

| 項目 | 内容 |
| --- | --- |
| `parse_ms` | FastInput の準備と入力を辺配列に読む時間 |
| `build_ms` | 辺配列からグラフを構築する時間。CSR では重みの変換も含む |
| `solve_ms` | `dijkstra` または `dijkstra_path` の呼び出し時間 |
| `recover_ms` | 全距離からの BFS による経路復元、または返された頂点列を辺列にする時間 |
| `output_ms` | 最短距離・経路の出力と flush |
| `whole_ms` | プロセス起動・入力・構築・計算・復元・出力・checksum・終了を含む時間 |

全距離と単一終点は求める範囲が違うため、結果を分けます。単一終点の `solve_ms` には `dijkstra_path` 内部の経路復元も含みます。`whole_ms` の測定用ドライバは辺配列も保持します。CSR では変換した辺配列も保持します。Library Checker に提出するドライバと同一の時間ではありません。

実行前にヘッダーを凍結し、全依存の SHA-256 を `environment.json` に記録します。CPU affinity を固定し、`/tmp/cp-wm-opt.lock` を共有して測定します。バイナリと入出力は Linux の `/tmp` に置き、各ケースで 1 回 warmup 後に順序を変えて別プロセスで実行します。`results.json` に全試行と中央値を保存します。

全距離は基準版と checksum を照合します。`--no-heap` を省略すると、標準 binary heap による独立実装も含めて照合します。単一終点は距離と経路を検証します。公式 checker と合成入力の経路検証は全体時間の計測後に実行します。`--kernel-only` は出力を省略し、経路検証を各区間の計測後に実行します。

| 合成入力 | 頂点数 | 辺数 | 内容 |
| --- | ---: | ---: | --- |
| `synthetic_line` | 500000 | 499999 | 重み 10^9 の一本道 |
| `synthetic_random` | 200000 | 500000 | 一本道とランダムな辺、重み 0〜10^9 |
| `synthetic_zero` | 200000 | 500000 | 全ての重みが 0 |
| `synthetic_binary` | 200000 | 500000 | 重みが 0 または 1 |
| `synthetic_dense` | 707 | 499142 | 有向完全グラフ、重み 0〜10^9 |
| `synthetic_relaxation` | 1000 | 499500 | 全ての辺で距離が改善される DAG |
| `synthetic_disconnected` | 500000 | 500000 | 先頭 100000 頂点だけが連結、終点は到達不能 |

生成 seed は `20261019` です。合成入力はアルゴリズムの比較用で、Library Checker の入力制約と同一とは限りません。

2026-10-19 に最速の [407097](https://judge.yosupo.jp/submission/407097)（38 ms）と [404592](https://judge.yosupo.jp/submission/404592)（40 ms）を調査しました。取得時の API 応答、各ケース時間、ソースの SHA-256 は `results/shortest_path_reference.json` にあります。API は当時のコンパイラ・フラグ・CPU を返さないため、上の judge 時間からローカルの速度を推定しません。参照ソースはキャッシュに保存し、リポジトリには含めません。

407097 は 32 bit 循環 radix heap・bucket ごとの最小値・32 件までの小配列を使います。404592 は 64 bit radix heap・bucket の存在 bitmask・AVX2 による最小値探索・32 件までの小配列を使います。どちらも CSR・専用入力・単一終点の打ち切りを使い、全出次数または全入次数が 1 以下のときの専用処理を持ちます。407097 はトポロジカルな前処理も含みます。

今回の既定 `dijkstra` は最小距離の確定順を保つキューを使い、`dijkstra_path` だけ終点の上界で打ち切ります。`<true>` はトポロジカルに取り除ける部分を先に緩和します。参照プログラムとは入力処理・辺順・同距離の取り出し順・専用分岐が異なるため、距離計算だけの比較と実提出コード全体の比較を分けます。

[最終検証](results/shortest_path_final_validation.json)では GCC / Clang の unit 計 8 件、ASan / UBSan 3 件、Library Checker 公式 29 ケース × 3 ドライバ × 2 コンパイラの計 174 件が成功しました。Boost を使う公開 sample 2 件と Fraction による比較 60 件も両コンパイラで成功しました。[内部キュー・CSR の単体検証](results/shortest_path_components_validation.json)と[初期検証](results/shortest_path_validation.json)の記録も残しています。

2026-10-19 の WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2、CPU 2 固定、`-O2 -DNDEBUG -march=native` で各 7 回の中央値です。追加フラグと全ソース hash は `results/shortest_path_native/environment.json` に記録しています。全 36 ケースで距離と経路を検証しました。

全頂点の距離計算時間です。隣接リストは従来と同じ `int64_t` 重み、CSR は `uint32_t` 重み・`int64_t` 距離です。

| 全距離 `solve_ms` | 変更前 | 変更後・隣接リスト | 変更後・CSR |
| --- | ---: | ---: | ---: |
| `line_00` | 115.21 | 92.51 | 21.62 |
| `almost_line_00` | 25.96 | 14.48 | 8.35 |
| `max_star_00` | 33.20 | 4.70 | 2.52 |
| `max_star_01` | 58.40 | 35.05 | 17.40 |
| `max_sparse_random_01` | 24.23 | 21.38 | 14.66 |
| `spfa_killer_00` | 23.86 | 13.86 | 8.67 |
| `wrong_dijkstra_killer_00` | 22.62 | 16.77 | 11.94 |
| `wrong_dijkstra_killer_01` | 34.77 | 26.01 | 14.01 |
| `synthetic_random` | 19.27 | 19.75 | 14.09 |
| `synthetic_zero` | 9.81 | 8.80 | 5.41 |
| `synthetic_relaxation` | 9.29 | 8.89 | 8.71 |

CSR の単一終点で、前処理を使う場合と使わない場合の比較です。前処理の固定費があるため、`<true>` が常に速いわけではありません。

| 単一終点 `solve_ms` | 既定 | `<true>` |
| --- | ---: | ---: |
| `line_00` | 32.99 | 39.65 |
| `max_sparse_random_00` | 0.58 | 4.61 |
| `sparse_random_01` | 1.08 | 9.90 |
| `wrong_dijkstra_killer_00` | 12.10 | 5.35 |
| `wrong_dijkstra_killer_01` | 16.85 | 16.14 |
| `synthetic_zero` | 1.83 | 3.95 |
| `synthetic_relaxation` | 8.42 | 0.64 |
| `synthetic_disconnected` | 7.40 | 4.53 |

実提出用コードの全体時間です。`own_path` は `test/dijkstra_path.test.cpp`、`own_topo` は `test/dijkstra_topological.test.cpp` です。最速参照との差は残っています。専用入力や一本道の専用処理の差も、全体時間に含まれます。

| 実提出 `whole_ms` | `own_path` | `own_topo` | 407097 | 404592 |
| --- | ---: | ---: | ---: | ---: |
| `line_00` | 59.52 | 68.26 | 25.64 | 21.38 |
| `almost_line_00` | 29.94 | 31.42 | 22.95 | 22.01 |
| `grid_random_00` | 14.40 | 15.03 | 10.62 | 8.83 |
| `max_dense_random_00` | 10.76 | 11.26 | 7.69 | 7.00 |
| `spfa_killer_00` | 30.15 | 34.08 | 22.95 | 20.47 |
| `wrong_dijkstra_killer_00` | 28.94 | 22.69 | 11.93 | 19.98 |
| `wrong_dijkstra_killer_01` | 39.71 | 38.08 | 26.68 | 26.73 |
| `synthetic_relaxation` | 19.91 | 11.89 | 6.75 | 12.81 |

同じ CPU で `-march=native` を外した GCC 15.2 でも、各 7 回測定しました。別ラウンドのため、それぞれの変更前後を比較します。全試行は `results/shortest_path_generic/` にあります。

| generic の全距離 `solve_ms` | 変更前 | 変更後・隣接リスト | 変更後・CSR |
| --- | ---: | ---: | ---: |
| `line_00` | 79.64 | 54.60 | 18.49 |
| `almost_line_00` | 23.53 | 14.49 | 8.75 |
| `max_star_00` | 31.37 | 4.59 | 2.90 |
| `spfa_killer_00` | 24.56 | 14.42 | 8.44 |
| `synthetic_random` | 20.51 | 19.05 | 13.63 |
| `synthetic_zero` | 11.45 | 9.53 | 5.24 |
| `synthetic_relaxation` | 9.08 | 7.52 | 7.72 |

小配列の上限を 32 件から 16 件へ下げる候補と、挿入キーが直前の確定値と等しいときに `bit_width` を省略する候補も比較しました。GCC 15.2、generic/native 各 3 回の比較です。いずれも全距離 checksum が一致しました。

| native の距離計算 | 上限 32 | 上限 16 | 同値キーを先に判定 |
| --- | ---: | ---: | ---: |
| `grid_random_00` | 8.77 | 8.40 | 9.72 |
| `spfa_killer_00` | 17.65 | 17.34 | 16.06 |
| `synthetic_line` | 4.17 | 5.34 | 4.65 |
| `synthetic_random` | 21.45 | 22.59 | 23.11 |
| `synthetic_zero` | 12.02 | 11.91 | 10.62 |
| `synthetic_binary` | 15.27 | 12.84 | 13.44 |
| `synthetic_relaxation` | 8.88 | 9.38 | 9.21 |

同値キーの分岐は全ゼロでは有効ですが、ランダムや一本道で遅くなりました。上限 16 も一本道で遅くなったため、上限 32 を維持しました。計測条件・全試行・当時の各候補のヘッダー 3 個・測定ドライバを `results/shortest_path_queue_variants/` に保存しています。ヘッダーとドライバの SHA-256 は当時の `environment.json` と一致します。

当時の候補を単独でコンパイルする例です。`queue32` を `queue16` または `queue_zero` に変えて比較できます。`synthetic_*.in` は上のスクリプト実行でキャッシュに生成されます。

```sh
base="$PWD/test/benchmark/results/shortest_path_queue_variants"
g++ -std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE \
  -ftrivial-auto-var-init=zero -march=native -I . \
  -DSP_BENCH_HEADER=\""$base/queue32/graph/dijkstra.hpp"\" \
  "$base/driver.cpp" -o /tmp/shortest-queue
taskset -c 2 /tmp/shortest-queue kernel \
  < .verify-helper/cache/shortest-path/benchmark/inputs/synthetic_zero.in
```
