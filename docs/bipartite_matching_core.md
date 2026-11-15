---
title: 二部マッチングの内部演算
documentation_of: //components/bipartite_matching_core.hpp
library_category: components/graph
---

通常は [二部マッチング](https://qandori.github.io/cp-library/graph/bipartite_matching.hpp.html) を使います。この部品は隣接頂点の保存、増加道探索、交互グラフの解析を共有します。名前空間は `bipartite_matching_detail` です。

CSR は、隣接頂点を 1 本の配列 `adj` へ並べ、各頂点の開始位置を `start` に持つ形式です。増加道は、マッチングに含まれない辺と含まれる辺を交互に通り、両端が未対応の頂点になる経路です。この経路で辺の採用・不採用を入れ替えると、本数を 1 増やせます。

$$N$$ は両側の頂点数の和、$$M$$ は辺数、$$K$$ は小さい側の頂点数、$$Q$$ は指定した左右の頂点リストの要素数の合計（重複を含む）です。

| 操作・型 | 内容 | 計算量 |
| --- | --- | --- |
| `CSR(n,edges,transpose)` | 頂点対から片方向の CSR を作る。`transpose` で左右を交換 | $$O(N+M)$$ |
| `CSR(n,graph)` | CSR の各辺を逆向きにした CSR を作る | $$O(N+M)$$ |
| `CSR::start` / `adj` | 行の開始位置／隣接頂点 | 参照 $$O(1)$$ |
| `hopcroft_karp(graph,left,right,phases,max_distance)` | 指定回数・距離まで増加道の段階を進める。上限を省略すると終了まで | $$O((N+M)\sqrt K)$$ |
| `Result::added` / `maximum` | 増加した本数／最大と確認できたか | $$O(1)$$ |
| `degree_greedy(graph,reverse,left,right,dl,dr,queue)` | 次数 1 の頂点を優先して既存解を改善し、増加した本数を返す | $$O(N+M)$$ |
| `warm_start(graph,left,right,multiplier)` | 作業量の上限内で既存のマッチングを改善し、増加した本数を返す | $$O((N+M)(1+\text{multiplier}))$$ |
| `augment(graph,left,right,multiplier,root_divisor,easy_phases)` | 両方式を組み合わせ、最大マッチングまで増やす | $$O((N+M)\sqrt K)$$（既定値） |
| `cover_side(edges,transpose,index,left,right)` | `index` で指定した側の全頂点を対応付けられれば真を返す | $$O((N+M)\sqrt K)$$ |
| `matching_covering(l,r,edges,required_left,required_right)` | 左右の指定頂点を全て対応付けるマッチングを `optional<vector<pair<int,int>>>` で返す。存在しなければ `nullopt` | $$O(Q+(N+M)\sqrt K)$$ |
| `Analysis(l,r,edges,left,right)` | 最大マッチングから交互グラフと到達範囲を作る | $$O(N+M)$$ |
| `Analysis::reach(queue,tail,start,adj,seen)` | 初期頂点からの到達判定 | $$O(N+M)$$ |
| `Analysis::components()` | 交互グラフの強連結成分を計算。計算済みなら省略 | 初回 $$O(N+M)$$、以降 $$O(1)$$ |
| `Analysis::from_left` / `to_right` | 未対応の左から到達／未対応の右へ到達できる頂点の印 | 参照 $$O(1)$$ |
| `Analysis::matched_edge` / `component` | 左ごとの採用辺 ID／頂点ごとの強連結成分番号 | 参照 $$O(1)$$ |
| `Analysis::left_size` | 左の頂点数 | $$O(1)$$ |
| `Analysis::start` / `adj` / `reverse_start` / `reverse_adj` | 交互グラフとその逆向きの CSR | 参照 $$O(1)$$ |

`left`・`right` は相手の番号を持つ配列で、未対応は -1 です。既存の有効なマッチングから開始できます。`hopcroft_karp` と `augment` は明示的なスタックを使い、増加道の長さに比例する再帰を行いません。`warm_start` の `multiplier` は非負です。作業量上限で停止した場合も有効なマッチングを維持し、残りは Hopcroft–Karp で処理できます。

`degree_greedy` の `reverse` は `graph` の逆向きの CSR です。`dl`・`dr`・`queue` は作業用の `std::vector<int>` で、それぞれ左頂点数・右頂点数・両側の頂点数の和の長さが必要です。内容は上書きされます。返り値は `int` で、マッチングの本数は減りません。最大とは限らないため、必要なら増加道探索を続けます。`warm_start` は `multiplier` が正ならこの処理も行い、0 ならマッチングを変更せず 0 を返します。

`cover_side` は指定した側だけを連番にして扱います。`transpose=false` では左、`true` では右が指定側です。`index` はその側の元の頂点番号を添字とし、対象外なら -1、対象なら $$[0,\text{left.size()})$$ の相異なる番号を入れます。`left` は対象頂点から反対側への対応、`right` は反対側から対象の連番への対応です。反対側の番号は元の番号を使います。有効な対応を初期値にでき、未対応は -1 です。偽を返した場合も有効な途中のマッチングを残します。

`matching_covering` の `edges` は左 $$[0,l)$$・右 $$[0,r)$$ の頂点対です。`required_left`・`required_right` は各側の元の頂点番号を並べた `span<const int>` で、重複も使えます。指定していない頂点を相手に使うこともあり、最大本数は保証しません。入力配列は変更しません。指定が空なら空の列を返します。上表の時間は $$K>0$$ の場合で、片側が空で指定がある場合は $$O(N+Q)$$ 時間・$$O(N)$$ 空間で `nullopt` を返します。

`Analysis` の左頂点番号は $$[0,l)$$、右頂点番号は $$[l,l+r)$$ です。辺は、採用した辺 ID だけ右から左、それ以外は左から右に向けます。多重辺も別々に扱います。

各処理の保存領域は $$O(N+M)$$、既定の `augment` の時間は $$O((N+M)\sqrt K)$$ です。
