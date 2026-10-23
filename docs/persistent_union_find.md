---
title: 永続 Union Find
documentation_of: //data_structure/persistent/persistent_union_find.hpp
redirect_from:
  - /data_structure/persistent_union_find.hpp.html
  - /lightweight/data_structure/persistent_union_find.hpp.html
---

過去版を残したまま連結成分を併合します。更新後も、以前の版の連結判定・成分サイズをオンラインで取得できます。更新の取り消しだけなら [Rollback Union Find](rollback_union_find.md) も使えます。

```cpp
#include "lib/data_structure/persistent/persistent_union_find.hpp"
#include <cassert>

int main() {
	PersistentUnionFind uf(5);
	auto v0 = uf.initial_root();
	auto v1 = uf.merge(v0, 0, 1);
	auto v2 = uf.merge(v1, 1, 2);
	auto v3 = uf.merge(v0, 3, 4);
	assert(!uf.same(v0, 0, 1) && uf.same(v2, 0, 2));
	assert(uf.size(v1, 0) == 2 && uf.size(v2, 0) == 3);
	assert(uf.component_count(v3) == 4);
}
```

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `PersistentUnionFind(n = 0)` | `n` 頂点、辺なしの初期版 | $$O(1)$$ |
| `merge(root,a,b)` | `a,b` の成分を併合した版。同一成分なら同じルート | $$O(\log^2 n)$$ |
| `leader(root,x)` | 成分の代表頂点 | $$O(\log^2 n)$$ |
| `same(root,a,b)` | 同じ成分か | $$O(\log^2 n)$$ |
| `size(root,x)` | `x` の成分の頂点数 | $$O(\log^2 n)$$ |
| `component_count(root)` | 成分数 | $$O(1)$$ |
| `vertex_count()` | 全頂点数 | $$O(1)$$ |
| `groups(root)` | 各成分の頂点リスト | $$O(n\log^2 n)$$ |

`0 <= n <= INT_MAX`、頂点は `0 <= x < n` とします。`root_type` は親配列のルートと成分数をまとめた値です。自分で中身を書き換えず、そのまま保存してください。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新の時間には領域確保の償却計算量を含みます。

初期構築は時間・領域とも $$O(1)$$、併合・代表取得・連結判定・成分サイズ取得は $$O(\log^2 n)$$ 時間です。併合 1 回の追加領域は $$O(\log n)$$ で、同一成分の併合はノードを作りません。長さ 0・1 の操作は定数時間です。
