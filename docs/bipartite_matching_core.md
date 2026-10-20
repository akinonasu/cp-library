---
title: 二部マッチングの内部演算
documentation_of: //components/bipartite_matching_core.hpp
---

通常は [二部マッチング](../graph/matching/bipartite_matching.hpp.html) を使います。この部品は CSR、増加道探索、交互グラフの解析を共有します。名前空間は `bipartite_matching_detail` です。

$$N$$ は両側の頂点数の和、$$M$$ は辺数、$$K$$ は小さい側の頂点数です。

| 操作・型 | 内容 | 計算量 |
| --- | --- | --- |
| `CSR(n,edges,transpose)` | 頂点対から片方向の CSR を作る。`transpose` で左右を交換 | $$O(N+M)$$ |
| `CSR(n,graph)` | CSR の各辺を逆向きにした CSR を作る | $$O(N+M)$$ |
| `CSR::start` / `adj` | 行の開始位置／隣接頂点 | 参照 $$O(1)$$ |
| `hopcroft_karp(graph,left,right,phases,max_distance)` | 指定回数・距離まで増加道の段階を進める。上限を省略すると終了まで | $$O((N+M)\sqrt K)$$ |
| `Result::added` / `maximum` | 増加した本数／最大と確認できたか | $$O(1)$$ |
| `warm_start(graph,left,right,multiplier)` | 距離ラベルで初期解を改善し、増加した本数を返す。作業量に上限を設ける | $$O((N+M)(1+\text{multiplier}))$$ |
| `augment(graph,left,right,multiplier,root_divisor,easy_phases)` | 両方式を組み合わせ、最大マッチングまで増やす | $$O((N+M)\sqrt K)$$（既定値） |
| `Analysis(l,r,edges,left,right)` | 最大マッチングから交互グラフと到達範囲を作る | $$O(N+M)$$ |
| `Analysis::reach(queue,tail,start,adj,seen)` | 初期頂点からの到達判定 | $$O(N+M)$$ |
| `Analysis::components()` | 交互グラフの強連結成分を計算。計算済みなら省略 | 初回 $$O(N+M)$$、以降 $$O(1)$$ |
| `Analysis::from_left` / `to_right` | 未対応の左から到達／未対応の右へ到達できる頂点の印 | 参照 $$O(1)$$ |
| `Analysis::matched_edge` / `component` | 左ごとの採用辺 ID／頂点ごとの強連結成分番号 | 参照 $$O(1)$$ |
| `Analysis::left_size` | 左の頂点数 | $$O(1)$$ |
| `Analysis::start` / `adj` / `reverse_start` / `reverse_adj` | 交互グラフとその逆向きの CSR | 参照 $$O(1)$$ |

`left`・`right` は相手の番号を持つ配列で、未対応は -1 です。既存の有効なマッチングから開始できます。`hopcroft_karp` と `augment` は明示的なスタックを使い、増加道の長さに比例する再帰を行いません。`warm_start` の `multiplier` は非負です。作業量上限で停止した場合も有効なマッチングを維持し、残りは Hopcroft–Karp で処理できます。

`Analysis` の左頂点番号は $$[0,l)$$、右頂点番号は $$[l,l+r)$$ です。辺は、採用した辺 ID だけ右から左、それ以外は左から右に向けます。多重辺も別々に扱います。

各処理の保存領域は $$O(N+M)$$、既定の `augment` の時間は $$O((N+M)\sqrt K)$$ です。
