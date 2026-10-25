---
title: General Matching（一般グラフ最大マッチング）
documentation_of: //graph/general_matching.hpp
redirect_from:
- /graph/matching/general_matching.hpp.html
- /lightweight/graph/matching/general_matching.hpp.html
library_category: graph/matching
---

```cpp
#include "lib/graph/general_matching.hpp"

GeneralMatching g(3);
g.add_edge(0, 1);
g.add_edge(1, 2);
int size = g.build(); // 1
auto pairs = g.edges();
auto partner = g.mate[0];
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `GeneralMatching(n)` | `n` 頂点の空グラフを構築 | $$O(n)$$ |
| `reserve(m)` | 入力辺の領域を予約 | $$O(m)$$ |
| `add_edge(u,v)` | 無向辺を追加 | 償却 $$O(1)$$ |
| `build()` | 最大マッチングを計算し、組数を返す | $$O(n^3+nm)$$ |
| `edges()` | マッチングの頂点対を返す | $$O(n)$$ |
| `mate[v]` | `build` 後の相手の頂点。なければ -1 | $$O(1)$$ |

Blossom 法で無向・重みなしグラフの最大マッチングを求めます。頂点は 0 始まりです。`build()` は組数、`edges()` は組の一覧を返し、`mate[v]` は相手の頂点（なければ `-1`）です。`reserve(m)` は辺の事前確保で、省略できます。

自己ループ・多重辺も渡せます。辺の追加後に再び `build()` できます。`mate` は参照用です。

時間は $$O(n^3+nm)$$、空間は $$O(n+m)$$ です。`edges()` は $$O(n)$$ です。
