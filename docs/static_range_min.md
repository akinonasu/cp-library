---
title: "Static Range Min"
documentation_of: //components/static_range_min.hpp
---

```cpp
#include "lib/components/static_range_min.hpp"

StaticRangeMin range(std::vector<int>{5, 2, 7, 1});
auto x = range.prod(0, 3); // 2
auto y = range.all_prod(); // 1
```

更新のない列の区間最小値を求めます。32 要素ごとの単調スタックをビットマスクで持ち、ブロック間には Sparse Table を使います。

| 操作 | 内容・返り値 |
| --- | --- |
| `StaticRangeMin<T>()` | 空の構造を作る。`T` の既定値は `int` |
| `StaticRangeMin(values)` | `vector<T>` から構築。型は推論可能 |
| `build(values)` | `vector<T>` から再構築 |
| `size()` | 要素数 |
| `empty()` | 空か |
| `operator[](i)` | 元の要素への const 参照。`T=bool` は値で返す |
| `prod(l,r)` | 非空の半開区間 `[l,r)` の最小値を返す |
| `all_prod()` | 列全体の最小値。空列には呼べない |

`T` には `<` による順序比較とコピーが必要です。デフォルト構築できない型にも対応します。`values` は値渡しで受け取り、左辺値ならコピー、`std::move(values)` ならムーブして保持します。長さは `INT_MAX` 以下としてください。コピー・ムーブに対応します。

比較・コピーを $$O(1)$$ として、長さを $$n$$、ブロック数を $$b=\lceil n/32\rceil$$ とすると、構築時間・空間は $$O(n+b\log b)$$、各参照操作は $$O(1)$$ です。空列の構築は $$O(1)$$ です。
