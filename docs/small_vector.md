---
title: "Small Vector"
documentation_of: //components/small_vector.hpp
---

```cpp
#include "lib/components/small_vector.hpp"
SmallVector<unsigned long long, 2> a;
a.push_back(123);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `SmallVector()` / `SmallVector(n)` / `SmallVector{...}` / `SmallVector(first,last)` | 空／長さ指定／初期値列／イテレータ範囲から構築 | 空: $$O(1)$$、要素指定: $$O(n)$$ |
| `size()` / `capacity()` / `empty()` | 要素数／確保容量／空判定 | $$O(1)$$ |
| `data()` / `begin()` / `end()` | 先頭ポインタ／先頭・末尾イテレータ | $$O(1)$$ |
| `operator[](i)` / `back()` | 要素／末尾の参照。const 版も利用可 | $$O(1)$$ |
| `reserve(n)` | 容量を確保 | $$O(n)$$ |
| `resize(n)` / `resize_for_overwrite(n)` | サイズ変更。前者は追加要素を 0 初期化、後者は初期化を省略 | $$O(n)$$ |
| `clear()` | 容量を残して空にする | $$O(1)$$ |
| `push_back(x)` / `pop_back()` | 末尾への追加／末尾の削除 | push_back: 償却 $$O(1)$$、pop_back: $$O(1)$$ |
| `assign(n,x)` / `assign(first,last)` | 同じ値を `n` 個／イテレータ範囲で置換 | $$O(n)$$ |
| `insert(pos,count,x)` / `insert(pos,first,last)` | `pos` の直前に複数要素を挿入し、挿入位置を返す | 移動・追加する要素数に比例 |
| `erase(first,last)` | ポインタ範囲を削除し、直後の要素の位置を返す | 移動する要素数に比例 |
| `a == b` / `a != b` | 要素列を比較 | $$O(n)$$ |

`SmallVector<T, N>` は先頭 $$N$$ 要素分を内部に持つ可変長配列です。`T` は trivially copyable かつ trivially destructible な型、$$N>0$$ が必要です。`size`・`capacity`・`empty`・`data`・`begin`・`end`・`[]`・`back`・`reserve`・`resize`・`clear`・`push_back`・`pop_back`・`assign`・`insert`・`erase` に対応します。範囲の `assign`・`insert` に自身の要素を渡さないでください。

`resize_for_overwrite(n)` は要素の初期化を省略してサイズを変更します。追加した要素は代入してから参照してください。

参照・末尾削除は $$O(1)$$、末尾追加は償却 $$O(1)$$、再確保・コピー・中間の挿入削除は $$O(n)$$、空間は $$O(\max(n,N))$$ です。
