---
title: 最小費用流
documentation_of: //graph/flow/min_cost_flow.hpp
redirect_from:
- /graph/min_cost_flow.hpp.html
---

```cpp
#include "lib/graph/flow/min_cost_flow.hpp"

MinCostFlow<> graph(3);
graph.add_edge(0, 1, 5, -2);
graph.add_edge(1, 2, 3, 7);
auto [flow, cost] = graph.flow(0, 2, 3);
```

| 操作・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `MinCostFlow<Cap,Cost,Total>(n=0)` | `n` 頂点で構築。既定は `Cap = std::int64_t`、`Cost = std::int64_t`、`Total = __int128_t` | $$O(N)$$ |
| `size()` / `edge_count()` | 頂点数／追加した辺数 | $$O(1)$$ |
| `reserve(m)` | 元の辺 `m` 本分の領域を確保 | $$O(M)$$ |
| `add_edge(from,to,cap,cost)` | 容量と単位費用を指定して有向辺を追加し、0 始まりの辺番号を返す | 償却 $$O(1)$$ |
| `flow(s,t,limit)` / `flow(s,t)` | `limit` 以下で可能な最大の流量と、その最小費用を `pair<Cap,Total>` で返す。省略時の上限は `Cap` の最大値 | 初期化＋ $$O(N+Mw+M(N+M\log N)\log U)$$ |
| `slope(s,t,limit)` / `slope(s,t)` | 流量と最小費用の折れ線の頂点を `vector<pair<Cap,Total>>` で返す。最初は `(0,0)`、傾きが同じ区間は統合する | 初期化＋ $$O(A(N+Mw))$$ |
| `get_edge(i)` | 追加時の順番で辺の情報を返す | $$O(1)$$ |
| `edges()` | 全ての辺の情報を `vector<Edge>` で返す | $$O(M)$$ |
| `reset_flow()` | 全ての流量を 0 に戻す。頂点と辺は残す | $$O(M)$$ |
| `potentials()` | 残余辺の reduced cost が非負となるポテンシャルを `const vector<Cost>&` で返す | 初期化が必要ならその時間、準備済み: $$O(1)$$ |
| `Edge` | `from`・`to`・`cap`・`flow`・`cost` を持つ | —（型） |
| `total_type` | 費用合計の型 `Total` | —（型） |

頂点は 0 始まりです。自己辺・多重辺・負の費用に対応します。**各呼び出し前の残余グラフ全体に負閉路がないこと**が必要です。負の辺がある場合も、初期ポテンシャルを求めてから処理します。負閉路を含む一般の循環流は、検証用の最小費用 b-flow のように、負費用辺を上限まで流した状態から逆向きの非負費用辺へ変換して扱えます。

`Cap` は 64 bit 以下の標準整数型で `bool` を除き、容量・上限は非負です。`Cost` は 64 bit 以下の標準の符号付き整数型、`Total` は符号付き整数型です。`cost` に `Cost` の最小値は指定できません。ポテンシャル・最短距離・reduced cost とそれらの計算途中は `Cost` に、費用合計と積は `Total` に収めてください。既定の合計値は [FastOutput](https://qandori.github.io/cp-library/components/fast_io.hpp.html) で出力できます。

`s != t` が必要です。頂点数は非負、元の辺数は `INT_MAX/2` 以下にします。繰り返し呼ぶと現在の残余グラフから追加分を流し、返り値の流量・費用は今回の追加分です。要求した量を全て流せたかは `flow == limit` で確認します。辺の追加後も続けられますが、負閉路が生じないことを確認してください。

逐次最短路法では、ポテンシャル付き Dijkstra と Radix Heap を使い、reduced cost が 0 の辺はヒープを通さず処理します。`flow` は増加回数が多い場合に Network Simplex へ切り替え、作業量の上限に達した場合は容量スケーリングを使います。`slope` は全ての折れ点を求めるため、逐次最短路法を使い続けます。ポテンシャルの参照は次の変更操作まで有効です。

$$N$$ 頂点・$$M$$ 辺、整数幅 $$w\leq64$$、増加回数 $$A$$、容量・流量上限の最大値を $$U\geq1$$ とします。初期化は非負費用からなら $$O(N+M)$$、一般には $$O(NM)$$。`slope` は初期化に加えて $$O(A(N+Mw))$$、`flow` は一定回数の逐次最短路法に加えて、切り替え時に $$O(M(N+M\log N)\log U)$$ 時間です。`potentials` は必要なら初期化を行い、それ以外は $$O(1)$$。辺追加は償却 $$O(1)$$、`get_edge`・値取得は $$O(1)$$、`edges`・`reset_flow`・領域を増やす `reserve` は $$O(M)$$、辺追加後の隣接配列の構築は $$O(N+M)$$、空間は $$O(N+M)$$、`slope` の出力は追加で $$O(A)$$ です。
