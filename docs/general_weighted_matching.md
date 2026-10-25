---
title: General Weighted Matching（一般グラフ最大重みマッチング）
documentation_of: //graph/general_weighted_matching.hpp
redirect_from:
- /graph/matching/general_weighted_matching.hpp.html
- /lightweight/graph/matching/general_weighted_matching.hpp.html
library_category: graph/matching
---

```cpp
#include "lib/graph/general_weighted_matching.hpp"

GeneralWeightedMatching graph(4);
graph.add_edge(0, 1, 1);
graph.add_edge(1, 2, 3);
graph.add_edge(2, 3, 1);
auto total = graph.build(); // 3
auto count = graph.matching_size(); // 1
auto pairs = graph.edges(); // { {1,2} }
auto partner = graph.mate[1]; // 2
```

無向一般グラフで、重みの合計が最大になるマッチングを求めます。重み付き Blossom 法を使い、奇数長の閉路も扱えます。頂点は `0..n-1` です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `GeneralWeightedMatching(n = 0)` | `n` 頂点の空グラフを作る | $$O(n^2)$$ |
| `size()` | 頂点数 | $$O(1)$$ |
| `add_edge(u,v,w)` | 重み `w` の無向辺を追加。同じ頂点対では最大の重みを使う | $$O(1)$$ |
| `build()` | 最大重みマッチングを計算し、重みの合計を `total_type` で返す。変更がなければ前回の結果を返す | 変更後: $$O(n^3)$$、変更なし: $$O(1)$$ |
| `weight()` | 計算済みの重みの合計 | $$O(1)$$ |
| `matching_size()` | 計算済みのマッチングの組数 | $$O(1)$$ |
| `edges()` | マッチングの頂点対を `vector<pair<int,int>>` で返す。各組は `u < v` | $$O(n)$$ |
| `mate[v]` | 相手の頂点。マッチしていなければ -1。参照用 | $$O(1)$$ |
| `clear()` | 頂点数・確保容量を保ち、全辺と計算結果を消す | $$O(n^2)$$ |
| `swap(other)` | 頂点数・辺・計算結果を交換 | $$O(1)$$ |
| `weight_type` | 辺重みの型。`int64_t` | —（型） |
| `total_type` | 重みの合計の型。`__int128_t` | —（型） |

最大化するのは重みの合計です。組数の最大化や、完全マッチングを要求する操作ではありません。同じ最大重みを持つ解が複数ある場合、どの解を返すかは未規定です。上の例では重み 3 の 1 組を選び、重み 1 の 2 組は選びません。

自己ループと重み 0 以下の辺は無視します。負の重みや `INT64_MIN` も渡せます。辺重みは `int64_t` の全範囲で使え、合計が 64 bit を超えても正しく計算します。内部の整数型は頂点数と最大重みに応じて 32／64／128 bit から選びます。

辺の追加後に再び `build()` できます。有効な辺を追加した後は、再計算するまで `weight()`・`matching_size()`・`edges()`・`mate` を使わないでください。構築直後・`clear()` 直後は空の結果を参照できます。`mate` は書き換えないでください。

コピー・ムーブに対応し、移動元は 0 頂点の空グラフになります。`n` は非負で `2*n+1` が int に収まる値としてください。重みは整数だけに対応します。

頂点数を $$n$$ とします。構築・`clear` は $$O(n^2)$$、辺の追加・`size`・`weight`・`matching_size`・`swap` は $$O(1)$$、`build` は $$O(n^3)$$ 時間です。グラフが変化していない場合の `build` は $$O(1)$$、`edges` は $$O(n)$$ 時間です。保持空間・計算中の追加空間はともに $$O(n^2)$$、`edges` の返り値は $$O(n)$$ 空間です。$$n=0$$ の操作は $$O(1)$$ です。
