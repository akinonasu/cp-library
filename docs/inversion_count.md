---
title: 転倒数
documentation_of: //sequence/inversion_count.hpp
---

数列の転倒数、すなわち $$i<j$$ かつ $$a_i>a_j$$ を満たす添字の組の個数を返します。同じ値の組は数えません。入力は変更しません。

```cpp
#include "lib/sequence/inversion_count.hpp"

std::vector<int> a{3, 1, 2, 1};
auto count = inversion_count(a);                     // 4
auto increasing = inversion_count(a, std::greater<>{}); // 1
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `inversion_count(a,compare = std::less<>{})` | `i < j` かつ `compare(a[j],a[i])` の組数を `std::int64_t` で返す | $$O(n\log n)$$ |

空列・長さ 1 の列は 0 です。比較関数は厳密弱順序を定めるものを使います。`std::greater<>{}` を指定すると、左の値が右の値より小さい組を数えます。比較関数で同値となる要素同士は、どちらの向きでも数えません。

入力は const なランダムアクセス・サイズ取得可能な range で、`vector`・`array`・`span`・`deque`・C 配列などを使えます。要素型に応じた比較関数を指定すれば、コピー不能な要素にも対応します。`vector<bool>` も扱えます。入力の長さは `INT_MAX` 以下とします。この範囲では最大の転倒数も符号付き 64 bit に収まります。

反復的なマージ処理で数え、座標圧縮や再帰は使いません。小さな単純コピー可能型では作業配列へ値をコピーし、それ以外では添字を並べ替えます。既に整列している場合は、作業配列を確保せずに 0 を返します。

長さを $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。既に整列している場合は $$O(n)$$ 時間・$$O(1)$$ 追加空間、空列は $$O(1)$$ 時間・空間です。比較とランダムアクセスを $$O(1)$$ とします。
