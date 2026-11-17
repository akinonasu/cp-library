---
title: 木のハッシュ
documentation_of: //tree/tree_hash.hpp
library_category: tree
---

根を持たない無向木全体の形のハッシュを求めます。頂点番号と隣接リストの順序に依存せず、頂点番号を付け替えると同じ辺集合になるか（木の同型）を調べるために使えます。

```cpp
#include "lib/tree/tree_hash.hpp"

std::vector<std::vector<int>> first{ {1}, {0, 2, 3}, {1}, {1} };
std::vector<std::vector<int>> second{ {3}, {3}, {3}, {0, 1, 2} };
auto a = tree_hash(first);
auto b = tree_hash(second);
bool same = a == b; // true: 頂点番号だけ異なる星
```

`tree_hash(graph,seed = 既定 seed)` は `std::array<TreeHashValue,2>` を返します。重心は、頂点を取り除いた後に残る連結成分の最大頂点数を最小にする頂点です。木の重心が 2 個あれば、それぞれを根にした木全体のハッシュを昇順に並べます。重心が 1 個なら同じハッシュを 2 個入れます。戻り値全体を `==` で比較してください。大小比較・ソートや `std::map` のキーとしても使えます。

入力は `std::vector<std::vector<int>>` の無向木です。頂点数は `INT_MAX` 以下とし、各辺を両方向に 1 回ずつ入れ、連結・自己ループなし・多重辺なしとしてください。空の木にも対応し、2 個のゼロ値を返します。入力を変更せず、返した値は入力の寿命に依存しません。

`TreeHashValue` は 2 本の 61 bit ハッシュを持ちます。既定 seed は [根付き木のハッシュ](rooted_tree_hash.md) と共通で、同じプロセス内では別々に構築した木同士を比較できます。値を保存して実行をまたいで比較する場合は、`tree_hash(graph,12345)` のように同じ seed を指定してください。

頂点番号を付け替えて同じ形になる木では重心同士も対応するため、同型な木の戻り値は必ず一致します。異なる木がハッシュ衝突する可能性はあります。ラベルを渡さない場合は頂点の値を区別せず、`tree_hash(graph,labels,seed = 既定 seed)` では対応する頂点のラベルも比較します。辺の値や重みは比較しません。根を指定して全ての部分木のハッシュを求める場合は [根付き木のハッシュ](rooted_tree_hash.md) を使ってください。

頂点ラベルは長さが頂点数と等しい列で、`labels[v]` が頂点 `v` の値です。`vector`・固定長／可変長の `span`・`array`・`string` など、定数時間の `size()` と添字アクセスが使える列に対応し、要素型は `bool` を含む 64 bit 以下の標準整数型にしてください。負数・最小値・最大値も扱えます。別々の木を比較するときは要素型と seed を揃え、同じ値を同じラベルとして渡してください。入力は変更せず、ラベル列を呼び出し後まで保持する必要はありません。空の木には空のラベル列を渡します。返り値はラベル無しと同じ `std::array<TreeHashValue,2>` ですが、ラベル付きとラベル無しの結果は比較しないでください。

```cpp
#include "lib/tree/tree_hash.hpp"
#include <cassert>

std::vector<std::vector<int>> first{ {1, 2}, {0}, {0} };
std::vector<std::vector<int>> second{ {1}, {0, 2}, {1} };
std::vector<int> first_labels{1000, -7, 257}, second_labels{257, 1000, -7};
assert(tree_hash(first, first_labels, 12345) == tree_hash(second, second_labels, 12345));
second_labels[0] = 999;
assert(tree_hash(first, first_labels, 12345) != tree_hash(second, second_labels, 12345));
```

ラベル付きでも、同じ形・同じ値の木なら必ず一致し、異なる木や値がハッシュ衝突する可能性はあります。

子の順序のソートや再帰は使いません。

頂点数を $$n$$ として、`tree_hash(graph,seed)`・`tree_hash(graph,labels,seed)` はともに $$O(n)$$ 時間・追加空間です。固定幅整数の演算は定数時間として扱います。返すハッシュは $$O(1)$$ 空間で、比較は $$O(1)$$ 時間です。
