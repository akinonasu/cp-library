---
title: "Fenwick Sum"
documentation_of: //components/fenwick_sum.hpp
---

```cpp
#include "lib/components/fenwick_sum.hpp"

FenwickSum<long long> bit(std::vector<long long>{3, 1, 4});
bit.add(1, 5);
auto sum = bit.prod(0, 3);
```

区間は $$[l,r)$$、添字は 0 始まりです。`T{}` が零元で、`+=`・`-=` が可換群の加減算になる型を使います。

| 操作 | 内容・返り値 |
| --- | --- |
| `FenwickSum()` / `FenwickSum(n)` | 空／零元が `n` 個の列を構築 |
| `FenwickSum(a)` / `build(a)` | 配列から構築／再構築 |
| `size()` | 要素数 |
| `add(i,x)` | 要素に `x` を加算 |
| `add_pair(i,x,j,y)` | 2 点に加算。共通の祖先はまとめて更新 |
| `prefix(r)` | $$[0,r)$$ の和 |
| `prod(l,r)` | 区間和。両端で共通する祖先の読み込みを省く |

要素数を $$n$$ として、構築・空間は $$O(n)$$、更新と和は $$O(\log n)$$、`size` は $$O(1)$$ です。`T` の演算は $$O(1)$$ とします。
