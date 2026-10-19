---
title: 最大流
documentation_of: //graph/flow/max_flow.hpp
redirect_from:
- /graph/max_flow.hpp.html
---

```cpp
#include "lib/graph/flow/max_flow.hpp"

MaxFlow<> graph(3);
graph.add_edge(0, 1, 5);
graph.add_edge(1, 2, 3);
auto flow = graph.flow(0, 2);
auto cut = graph.min_cut(0);
```

| 操作・型 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `MaxFlow<Cap>(n=0)` | `n` 頂点で構築。`Cap` の既定は `std::int64_t` | $$O(N)$$ |
| `size()` / `edge_count()` | 頂点数／追加した辺数 | $$O(1)$$ |
| `reserve(m)` | 元の辺 `m` 本分の領域を確保 | $$O(M)$$ |
| `add_edge(from,to,cap)` | 有向辺を追加し、0 始まりの辺番号を返す | 償却 $$O(1)$$ |
| `flow(s,t)` | 最大流まで流し、今回追加した流量を返す。Highest Label Push-Relabel を使う | $$O(N^2M)$$、初回は別途 $$O(N+M)$$ |
| `flow(s,t,limit)` | 今回の流量を `limit` 以下に制限し、追加した流量を返す。反復版 Dinic を使う | $$O(N^2M)$$、初回は別途 $$O(N+M)$$ |
| `min_cut(s)` | 残余グラフで `s` から到達できるかを `vector<bool>` で返す | $$O(N+M)$$ |
| `get_edge(i)` | 追加時の順番で辺の情報を返す | $$O(1)$$ |
| `edges()` | 全ての辺の情報を `vector<Edge>` で返す | $$O(M)$$ |
| `change_edge(i,cap,flow)` | 辺の容量と流量を変更。`0 <= flow <= cap` が必要 | $$O(1)$$ |
| `reset_flow()` | 全ての流量を 0 に戻す。頂点と辺は残す | $$O(M)$$ |
| `Edge` | `from`・`to`・`cap`・`flow` を持つ | —（型） |

頂点は 0 始まりです。自己辺・多重辺・容量 0 に対応します。`Cap` は 64 bit 以下の標準整数型で、`bool` を除きます。容量・流量上限は非負、返す流量と各辺の容量は `Cap` に収めます。Push-Relabel の途中の余剰流量は 128 bit で持つため、始点から出る容量の総和が `Cap` を超える場合も扱えます。

`s != t` が必要です。頂点数は `INT_MAX/2-1` 以下、元の辺数は `INT_MAX/2` 以下にします。再帰を使わないため、長いパスも扱えます。

残余グラフを保持するため、同じ始点・終点で繰り返し呼ぶと続きを流します。辺を追加してから続けることもできます。`change_edge` で現在のフローを変更する場合は、始点・終点以外の流量保存則を保ってください。最小カットが必要な場合は、`flow(s,t)` で最大流まで流した後に `min_cut(s)` を呼びます。

$$N$$ 頂点・$$M$$ 辺として、構築は $$O(N)$$ 以下、辺追加は償却 $$O(1)$$、最大流はどちらも一般の上界が $$O(N^2M)$$、`min_cut` は $$O(N+M)$$ です。辺追加後の最初の流量計算・`min_cut` には隣接配列の構築 $$O(N+M)$$ が加わります。`get_edge`・値取得・`change_edge` は $$O(1)$$、`edges`・`reset_flow` は $$O(M)$$、領域を増やす `reserve` は $$O(M)$$、空間は $$O(N+M)$$ です。
