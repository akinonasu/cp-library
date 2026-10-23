---
title: Sliding Window Aggregation
documentation_of: //data_structure/sliding_window_aggregation.hpp
---

末尾への追加・先頭の削除ができるキューです。キュー全体を順番どおりに集約した値を取得できます。非可換な演算にも対応します。

```cpp
#include "lib/data_structure/sliding_window_aggregation.hpp"
#include <string>

std::string op(std::string a, std::string b) { return a + b; }
std::string e() { return ""; }

SlidingWindowAggregation<std::string, op, e> queue;
queue.push_back("ab");
queue.push_back("cd");
auto a = queue.prod(); // "abcd"
queue.pop_front();
auto b = queue.front(); // "cd"
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SlidingWindowAggregation<S,op,e>()` | 空のキューを作成 | $$O(1)$$ |
| `SlidingWindowAggregation(values)` | `vector<S>` から順番に追加 | $$O(n)$$ |
| `push_back(value)` | 末尾に追加 | 償却 $$O(1)$$ |
| `pop_front()` | 先頭を削除 | 償却 $$O(1)$$、最悪 $$O(n)$$ |
| `front()` / `back()` | 先頭／末尾の値の `const S&` | $$O(1)$$ |
| `prod()` | 先頭から末尾まで集約。空なら `e()` | $$O(1)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `reserve(n)` | 領域を確保 | $$O(n)$$ |
| `clear()` | 全要素を削除 | $$O(n)$$ |
| `swap(other)` | 内容を交換 | $$O(1)$$ |

`op(S,S)` は結合的な演算、`e()` はその単位元です。`S` はコピー・移動できる型を使います。演算の引数は変更しないでください。`front`・`back`・`pop_front` はキューが空でないときに使います。取得操作は `const` オブジェクトから呼べます。

コピーは独立したキューを作り、移動元は再利用できる空のキューになります。参照は更新後に取得し直してください。`size_type` は `std::size_t` です。

要素のコピーと演算を定数時間としたとき、追加・削除は償却 $$O(1)$$、集約は $$O(1)$$ 時間です。確保した領域を $$C$$ として使用空間は $$O(C)$$、コピーは $$O(n)$$ 時間です。文字列結合のように演算自体に時間がかかる場合は、その費用が加わります。
