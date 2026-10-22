---
title: "Union-Find"
documentation_of: //data_structure/union_find.hpp
---

```cpp
#include "lib/data_structure/union_find.hpp"

UnionFind uf(5);
uf.merge(0, 2);
bool connected = uf.same(0, 2); // true
int vertices = uf.size(0); // 2
int components = uf.count(); // 4
auto groups = uf.groups();
```

頂点は 0 始まりです。構築後の頂点数は固定です。`merge(a,b)` は併合後の代表元を返し、既に同じ成分なら変更しません。`leader(v)` は代表元、`size(v)` は所属成分の頂点数、引数なしの `size()` は全頂点数、`count()` は成分数を返します。

`groups()` は代表元の昇順に成分を並べ、各成分の頂点を昇順に返します。経路短縮で内部状態を更新するため、検索操作と `groups()` は非 const です。コピーは独立します。頂点数は非負の `int`、頂点引数は範囲内であることが必要です。

サイズによる併合と反復的な経路半減を使い、`groups()` は実在する成分の分だけ外側の配列を確保します。

頂点数を $$n$$ として、構築・空間は $$O(n)$$、併合・検索・成分サイズは償却 $$O(\alpha(n))$$、全頂点数と成分数は $$O(1)$$、`groups()` は $$O(n\alpha(n))$$ 時間・$$O(n)$$ 追加空間です。
