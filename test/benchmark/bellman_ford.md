# 負辺の最短路

2026-10-20。一般の疎な有向グラフを対象に、`bellman_ford` と `goldberg_radzik` を追加した。経路と到達可能な負閉路を、実際に使った辺を含めて復元する。

## 調査

- [Bringmann–Cassis–Fischer, FOCS 2023](https://arxiv.org/abs/2304.05279) は整数重みに対する乱択法で $$O(m\log^2 n\log(nW)\log\log n)$$ を達成する。ここで $$W$$ は最も小さい負辺の重みの絶対値の上界。これは今回の実装の計算量ではない。
- [Cassis et al., SEA 2025](https://doi.org/10.4230/LIPIcs.SEA.2025.10) はその方向の実装を Goldberg–Radzik と比較している。実験結果は入力に依存するため、理論上の改善から競プロで常に高速になるとは判断しない。著者の公開実装に含まれる `gor.c`・`gor.cpp` も調べた。
- [NetworkX の Goldberg–Radzik 実装](https://networkx.org/documentation/stable/_modules/networkx/algorithms/shortest_paths/weighted.html#goldberg_radzik) を調べた。本実装では厳密に改善する辺をたどる反復 DFS、後順序の逆順での緩和、DFS 中の負閉路検出を使う。辺の添字も記録して、多重辺を含む閉路を復元する。

取得日時・公開実装の commit・ソースの SHA-256 は `results/bellman_ford/reference.json` にある。研究実装をそのままローカルで動かした比較は行っていない。

Library Checker の Shortest Path の AC 実行時間順の先頭 3 件も取得した。

| 提出 | 取得時の時間 | 比較した手法 |
| --- | ---: | --- |
| [407097](https://judge.yosupo.jp/submission/407097) | 38 ms | CSR、整数 radix heap、一意経路・トポロジカル処理、目標頂点向けの省略 |
| [404592](https://judge.yosupo.jp/submission/404592) | 40 ms | CSR、64 bit radix heap、SIMD を使う bucket 内の最小値取得 |
| [403027](https://judge.yosupo.jp/submission/403027) | 40 ms | 同系統の radix heap と非負辺の経路探索 |

この問題の辺は非負なので、負辺アルゴリズム自体の速度比較には使えない。参考にしたのは CSR・整数ヒープ・不要な探索の省略であり、今回も既存の `DirectedCSR` と `RadixHeap` を利用する。負辺の正当性は AOJ GRL_1_B と独立した Floyd–Warshall で検証した。

## 採用した構成

1. 通常の緩和走査を最大 2 回行う。更新がなければその場で終了する。
2. 続く場合は、始点から到達可能な部分だけを反復 Tarjan 法で強連結成分に分解する。
3. 成分をトポロジカル順に処理する。成分の外から来る距離は、内部の処理に入る時点で確定している。
4. 内部に負辺がなければ多始点 Dijkstra を使う。初期距離が負でも、内部の辺が非負なら使える。64 bit 以下では最小の初期距離を下限とする radix heap、128 bit では比較ヒープを使う。
5. 内部に負辺があれば Goldberg–Radzik 法で処理する。成分の処理を終えてから、後の成分への辺を緩和する。

前段の 2 回の走査は、実際の経路による距離の上界を作る。以降もその上界から緩和を続けられる。成分の間は DAG なので、未処理成分から処理済み成分へ距離が戻ってくることはない。

Goldberg–Radzik では更新された頂点から DFS を始め、順序を整えて緩和する。DFS 中に探索中の祖先へ厳密な改善が起きた場合は、その DFS 木上の区間と辺から負閉路を返す。自己辺や平行な辺も、隣接リスト内の添字で区別する。再帰は使わない。

既定の 64 bit の距離型で、最悪時間は $$O(n+nm)$$。DAG では $$O(n+m)$$。全ての疎グラフでほぼ線形になる保証はない。前処理なしの `goldberg_radzik` も公開し、成分分解が不要な場合に選べるようにした。

## 比較した前段

最初に、SLF キューの走査量を $$4m+n$$ で打ち切る方式を測定した。しかし、キュー処理と切り替え後の計算が重なり、乱数の疎グラフでは 8.828 ms、成分分解だけでは 4.143 ms だった。

次に、前段なし・SLF・連続走査 1 回・2 回・4 回を各 3 回測定した。`variants/summary.json` がその結果で、測定したソースは `variants/sources.tar.gz` に保存した。2 回の走査は、単純な走査で短時間に収束する場合の速度と、収束しない場合の余分な走査量の兼ね合いで選んだ。入力ファイル名・頂点番号の特定の並び・ジャッジ固有の値に応じた分岐は入れていない。

## 最終測定

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定し、他のコンパイル・テスト・ベンチマークとは同時に測定しない。
- 生成・CSR 構築・結果の checksum と入出力を計測外にする。距離・復元用配列の確保と、最短路の計算を計測する。
- 全方式で 64 bit 整数の重み・距離、同じ CSR、範囲を検査する加算を使う。基準実装も経路用の親と辺番号を保持する。
- seed `20261020`、各 7 回。実行順を固定 seed で入れ替え、中央値を採る。全距離の checksum が全試行で一致することを確認する。
- 比較は通常の全辺走査 `basic`、FIFO の SPFA、SLF の SPFA、成分分解なしの Goldberg–Radzik `gor`、前段なしの成分分解 `scc`、採用した 2 回の走査付き `library`。

| 入力 | 頂点数 / 辺数 | 内容 |
| --- | ---: | --- |
| sparse | 20,000 / 100,000 | 重み 1 の環と乱数の非負辺に、頂点ごとの乱数ポテンシャルの差を加える。負辺はあるが負閉路はない |
| dense | 3,000 / 300,000 | 上と同じ生成方法で密度を上げる |
| dag | 6,000 / 23,996 | 頂点番号を入れ替えた負辺の一本道と、前方への大きな重みの辺 |
| components | 20,000 / 39,996 | 4 頂点ずつの環にポテンシャル差を加え、成分を負辺でつなぐ |
| positive_bridge | 20,000 / 100,000 | 始点からの負辺と、その先の非負辺成分 |
| augmented_gor | 4,001 / 26,003 | 論文の BAD-GOR を参考にした形に、負閉路を作らない大きな正の戻り辺・乱数辺を追加する |

辺の順序・頂点番号によって走査型の速度も変わる。特に `sparse` は環の順序が頂点番号順で、単純な走査にも有利なケース。

generic、単位 ms。

| 入力 | basic | FIFO | SLF | gor | scc | 本実装 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| sparse | 2.270 | 6.661 | 7.252 | 3.160 | 4.127 | 2.618 |
| dense | 3.051 | 3.075 | 3.146 | 3.148 | 4.411 | 2.831 |
| dag | 276.073 | 131.262 | 2.981 | 0.344 | 0.381 | 0.412 |
| components | 241.732 | 233.975 | 381.510 | 7.099 | 0.965 | 1.232 |
| positive_bridge | 2.373 | 6.292 | 2.936 | 3.226 | 2.209 | 2.757 |
| augmented_gor | 0.158 | 40.009 | 0.755 | 0.838 | 1.059 | 0.168 |

native、単位 ms。

| 入力 | basic | FIFO | SLF | gor | scc | 本実装 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| sparse | 2.166 | 6.580 | 7.198 | 3.485 | 4.595 | 2.666 |
| dense | 3.116 | 3.089 | 3.243 | 3.441 | 4.609 | 2.831 |
| dag | 253.393 | 119.483 | 2.591 | 0.412 | 0.363 | 0.383 |
| components | 256.405 | 244.733 | 398.317 | 10.341 | 1.123 | 1.304 |
| positive_bridge | 2.293 | 5.839 | 2.673 | 3.466 | 2.137 | 2.651 |
| augmented_gor | 0.146 | 32.795 | 0.647 | 0.847 | 1.048 | 0.150 |

全入力で最速にはならない。今回の疎な乱数グラフでは単純な走査より約 15～23% 遅く、成分分解そのものが最も速いケースもある。一方、`components` は単純な走査や SPFA が遅くなる構造を成分ごとに処理できる。公開した `goldberg_radzik` だけの方が速い入力もあるため、測定結果とともに選択肢を残した。

条件とソースの SHA-256 は各 `environment.json`、全試行は `samples.csv`、中央値は `summary.json` にある。AtCoder 実機・macOS 実機での速度は未測定。

## 検証

- GCC 15.2 / Clang 21.1.8 の単体ヘッダー・単体テスト・既存ライブラリとの同時 include。
- 独立した Floyd–Warshall と、既定版・成分分解だけの版・Goldberg–Radzik 版を比較。通常の隣接リストと CSR の両方を使い、全頂点を始点として検証。
- 負辺を直接乱数で生成したグラフ、ポテンシャルで負閉路がないことを保証したグラフ、DAG、自己辺、多重辺、重み 0 の閉路、到達不能な負閉路。
- 全有限距離・到達判定・復元経路の辺と重み・復元負閉路の辺と重み・到達可能部分の強連結成分を照合。
- 20 万頂点の昇順と逆順の一本道、長い重み 0 の閉路、長い負閉路。反復 DFS が必要なケースも検証。
- 負の初期距離を持つ非負辺成分、radix heap の符号境界、128 bit の距離と比較ヒープ、加算範囲外の例外。
- Clang ASan / UBSan。
- GCC / Clang の両方で AOJ GRL_1_B の公式 40 ケース、Library Checker Shortest Path の公式 29 ケースに正答。最短路の非一意な出力は距離と実際の辺で検証。
- 両ヘッダーの Bundle、ドキュメント例のコンパイルと実行。

`check.log`・`sanitize.log`・`validation.log`・`validation.json` に結果を記録した。macOS 実機のローカル検証は行っておらず、macOS CI の対象。

## 再現

```sh
python3 test/benchmark/bellman_ford_reference.py
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/bellman_ford/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/bellman_ford_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/bellman_ford.py --output test/benchmark/results/bellman_ford/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/bellman_ford.py --native --output test/benchmark/results/bellman_ford/reproduced_native
```
