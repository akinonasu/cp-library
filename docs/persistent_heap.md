---
title: 永続併合可能ヒープ
documentation_of: //data_structure/persistent/persistent_heap.hpp
redirect_from:
  - /data_structure/persistent_heap.hpp.html
  - /lightweight/data_structure/persistent_heap.hpp.html
---

過去版を残して要素の追加・最小値の削除・ヒープ同士の併合を行います。既定は最小ヒープです。最大ヒープには `PersistentHeap<T, std::greater<T>>` を使います。

```cpp
#include "lib/data_structure/persistent/persistent_heap.hpp"
#include <cassert>

int main() {
	PersistentHeap<int> heap;
	auto a = heap.push(heap.initial_root(), 5);
	auto b = heap.push(a, 2);
	auto c = heap.push(a, 8);
	auto d = heap.meld(b, c);
	assert(heap.size(d) == 4 && heap.top(d) == 2);
	assert(heap.top(heap.pop(d)) == 5);
	assert(heap.top(a) == 5 && heap.top(b) == 2);
}
```

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `PersistentHeap<T,Compare>(compare = Compare{})` | 空ヒープ。既定は `std::less<T>` | $$O(1)$$ |
| `size(root)` / `empty(root)` | 要素数／空か | $$O(1)$$ |
| `top(root)` | 比較器で最初の値。非空が必要 | $$O(1)$$ |
| `push(root,x)` | `x` を追加した版 | $$O(\log n)$$ |
| `pop(root)` | 先頭の値を 1 個削除した版。非空が必要 | $$O(\log n)$$ |
| `meld(a,b)` | 2 つのヒープの全要素を含む版 | $$O(\log(n+m))$$ |

同じオブジェクトの版同士だけを併合してください。重複や共有部分もそれぞれ 1 回ずつ数えるため、同じ版を自分自身と併合すると個数は 2 倍になります。個数は `std::int64_t` で、併合後も `INT64_MAX` 以下にしてください。`Compare` は狭義弱順序、`T` はコピー可能な型とします。`T` のデフォルト構築は不要です。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新の時間には領域確保の償却計算量を含みます。

push・pop は $$O(\log n)$$、meld は $$O(\log(n+m))$$ 時間で、同じオーダーの追加領域を使います。top・size は $$O(1)$$ です。空ヒープ・要素 1 個の操作は定数時間です。読み取りはノードを作りません。
