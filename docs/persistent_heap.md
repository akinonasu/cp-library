---
title: 永続併合可能ヒープ
documentation_of: //data_structure/persistent_heap.hpp
redirect_from:
- /data_structure/persistent/persistent_heap.hpp.html
- /lightweight/data_structure/persistent/persistent_heap.hpp.html
library_category: data_structure/persistent
---

過去のバージョンを残して要素の追加・最小値の削除・ヒープ同士の併合を行います。既定は最小ヒープです。最大ヒープには `PersistentHeap<T, std::greater<T>>` を使います。

```cpp
#include "lib/data_structure/persistent_heap.hpp"
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

各バージョンは `root_type` の値（ルート）で指定します。各更新は新しいバージョンのルートを返し、引数で指定したバージョンは変更しません。`initial_root()` が最初のバージョンです。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去のバージョンから分岐できます。同じオブジェクトから得たルートだけを渡してください。バージョンごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `PersistentHeap<T,Compare>(compare = Compare{})` | 空ヒープ。既定は `std::less<T>` | $$O(1)$$ |
| `size(root)` / `empty(root)` | 要素数／空か | $$O(1)$$ |
| `top(root)` | 比較器で最初の値。非空が必要 | $$O(1)$$ |
| `push(root,x)` | `x` を追加したバージョン | 償却 $$O(\log n)$$ |
| `pop(root)` | 先頭の値を 1 個削除したバージョン。非空が必要 | 償却 $$O(\log n)$$ |
| `meld(a,b)` | 2 つのヒープの全要素を含むバージョン | 償却 $$O(\log(n+m))$$ |

同じオブジェクトのバージョン同士だけを併合してください。重複や共有部分もそれぞれ 1 回ずつ数えるため、同じバージョンを自分自身と併合すると個数は 2 倍になります。個数は `std::int64_t` で、併合後も `INT64_MAX` 以下にしてください。`Compare(a,b)` は `a` が `b` より前に来るかを返します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前となるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。`T` はコピー可能な型とします。`T` のデフォルト構築は不要です。

`reserve(count)` は全バージョンを合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新時間は領域確保を含む償却計算量です。

全バージョンを合わせた作成済みノード数を $$K$$、確保容量を $$C$$ とすると、保持空間は $$O(C)$$、オブジェクト自体のコピーは $$O(K)$$ 時間・追加空間です。コピー代入には、代入先の旧ノードを破棄する時間も加わります。`initial_root()`・`node_count()`・`memory_usage()` は $$O(1)$$、`reserve(count)` は容量が十分なら $$O(1)$$、再確保時は $$O(K)$$ 時間です。作成済みノード数と `count` は $$2^{32}-2$$ 以下とし、ノードがない場合にもコピー・予約には定数時間かかります。

push・pop は $$O(\log n)$$、meld は $$O(\log(n+m))$$ 時間で、同じオーダーの追加領域を使います。top・size は $$O(1)$$ です。空ヒープ・要素 1 個の操作は定数時間です。読み取りはノードを作りません。
