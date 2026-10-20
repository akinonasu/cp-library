---
title: 64 bit キーの安定基数ソート
documentation_of: //components/radix_sort.hpp
---

```cpp
#include "lib/components/radix_sort.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
	struct Item { std::uint64_t key; int id; };
	std::vector<Item> values = { {5, 0}, {1, 1}, {5, 2} };
	stable_radix_sort(values, [](const Item &item) { return item.key; });
	assert(values[0].id == 1 && values[1].id == 0 && values[2].id == 2);
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `stable_radix_sort(values,key)` | `vector<T>` を `key(const T&)` の昇順に並べ替え。同じキーでは元の順序を保つ | $$O(N)$$ |

キーは64 bit以下の符号なし整数にしてください。要素はデフォルト構築・ムーブ構築・ムーブ代入が必要です。コピーできない要素も利用できます。キーの関数は要素を変更せず、呼び出すたびに同じ値を返すものを使ってください。

補助配列との入れ替えにより、元の要素への参照・ポインタ・イテレータは無効になる場合があります。

8 bitずつの安定な分配を行います。全要素で共通のバイトは処理を省き、既に昇順なら補助配列を確保しません。256要素未満では `std::stable_sort` を使います。

要素数を $$N$$ として、時間は $$O(N)$$、追加空間は $$O(N)$$ です。キーの計算・要素の構築・移動は $$O(1)$$ とします。空の場合にも定数時間かかります。
