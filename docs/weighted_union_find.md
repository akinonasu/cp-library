---
title: 重み付き Union-Find
documentation_of: //data_structure/weighted_union_find.hpp
redirect_from:
- /data_structure/union_find/weighted_union_find.hpp.html
- /lightweight/data_structure/union_find/weighted_union_find.hpp.html
library_category: data_structure/union_find
---

各頂点に対応する値を `potential` と書き、頂点どうしの値の差を管理します。絶対的な値を決めずに、「頂点 `y` の値が頂点 `x` より `w` 大きい」という関係を追加できます。

```cpp
#include "lib/data_structure/weighted_union_find.hpp"

WeightedUnionFind<std::int64_t> uf(3);
bool ok = uf.merge(0, 1, 5); // true
auto value = uf.diff(0, 1); // 5
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WeightedUnionFind<T>(n)` | `n` 頂点を独立した成分として構築 | $$O(n)$$ |
| `merge(x,y,w)` | $$\mathrm{potential}[y]-\mathrm{potential}[x]=w$$ を追加。矛盾時に false | 償却 $$O(\alpha(n))$$ |
| `same(x,y)` | 連結判定 | 償却 $$O(\alpha(n))$$ |
| `leader(x)` / `size(x)` | 代表元／連結成分の頂点数 | 償却 $$O(\alpha(n))$$ |
| `weight(x)` | 代表頂点の値を 0 としたときの `x` の値 | 償却 $$O(\alpha(n))$$ |
| `diff(x,y)` | 連結な頂点間の $$\mathrm{potential}[y]-\mathrm{potential}[x]$$ | 償却 $$O(\alpha(n))$$ |

`merge(x, y, w)` は $$\mathrm{potential}[y]-\mathrm{potential}[x]=w$$ を追加し、矛盾する場合に `false` を返します。`diff(x, y)` は連結な頂点間の $$\mathrm{potential}[y]-\mathrm{potential}[x]$$ を返します。

`same(x, y)` で連結判定、`leader(x)` で代表元、`size(x)` で連結成分の頂点数を取得できます。頂点番号は 0 始まりです。

例では頂点 1 の値が頂点 0 より 5 大きいという関係を登録しています。絶対的な値を指定する必要はありません。`diff` の前に `same` で連結を確認してください。既存の関係と矛盾する `merge` は false を返し、関係を変更しません。

頂点数を $$n$$ として、構築・空間は $$O(n)$$、各操作は償却 $$O(\alpha(n))$$ です。$$\alpha$$ は逆アッカーマン関数で、増加が非常に遅く、実用的な頂点数では 5 以下です。`T` の演算・コピーは $$O(1)$$ とします。

「償却」は、一連の操作にかかる合計時間から求めた、1 回あたりの計算量を表します。
