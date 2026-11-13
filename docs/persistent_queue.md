---
title: 永続キュー
documentation_of: //data_structure/persistent_queue.hpp
redirect_from:
- /data_structure/persistent/persistent_queue.hpp.html
- /lightweight/data_structure/persistent/persistent_queue.hpp.html
library_category: data_structure/persistent
---

過去版からの末尾追加・先頭削除・先頭取得を行います。任意の位置の要素も取得できます。両端の更新や列の分割・連結には [永続 Deque](persistent_deque.md) を使います。

```cpp
#include "lib/data_structure/persistent_queue.hpp"
#include <cassert>

int main() {
	PersistentQueue<int> queue;
	auto v0 = queue.initial_root();
	auto v1 = queue.push(v0, 10);
	auto v2 = queue.push(v1, 20);
	auto v3 = queue.pop(v2);
	auto v4 = queue.push(v1, 30);
	assert(queue.front(v3) == 20 && queue.front(v2) == 10);
	assert(queue.back(v4) == 30 && queue.get(v4, 1) == 30);
}
```

保存した各状態を版と呼び、その状態を指定する `root_type` の値をルートと呼びます。各更新は新しい版のルートを返し、引数で指定した版は変更しません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `size(root)` / `empty(root)` | 要素数／空か | $$O(1)$$ |
| `push(root,x)` | `x` を末尾に追加した版 | 償却 $$O(1)$$ |
| `pop(root)` | 先頭を削除した版。非空が必要 | $$O(1)$$ |
| `front(root)` | 先頭要素。非空が必要 | $$O(\log u)$$ |
| `back(root)` | 末尾要素。非空が必要 | $$O(1)$$ |
| `get(root,p)` | 先頭から 0 始まり `p` 番目の要素 | $$O(\log u)$$ |

`get` は `0 <= p < size(root)` とします。`u` は空になるまでの同じ分岐上での push 回数です。`u <= INT_MAX` とし、`T` はコピー可能な型とします。`T` のデフォルト構築は不要です。`root_type` のフィールドは書き換えず、そのまま保存してください。

`front_many(roots)` は `std::vector<root_type>` で渡した非空の版それぞれの先頭要素を、指定順の `std::vector<T>` として返します。同じ版を複数回指定できます。空の版リストには空の列を返します。版の数は `INT_MAX` 以下とします。版や格納済みのノードは変更しません。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。時間には領域を確保し直す費用も含め、一連の更新の合計から 1 回あたりの上限を求めています（償却計算量）。

全版を合わせた作成済みノード数を $$K$$、確保容量を $$C$$ とすると、保持空間は $$O(C)$$、オブジェクト自体のコピーは $$O(K)$$ 時間・追加空間です。コピー代入には、代入先の旧ノードを破棄する時間も加わります。`initial_root()`・`node_count()`・`memory_usage()` は $$O(1)$$、`reserve(count)` は容量が十分なら $$O(1)$$、再確保時は $$O(K)$$ 時間です。作成済みノード数と `count` は $$2^{32}-2$$ 以下とし、ノードがない場合にもコピー・予約には定数時間かかります。

push は償却 $$O(1)$$、pop・back・size は $$O(1)$$、front・添字アクセスは $$O(\log u)$$ 時間です。push 1 回につき 1 ノードを追加します。他の操作はノードを作りません。過去版から何度分岐してもこれらの計算量を保ちます。空または 1 ノードの履歴に対する操作は定数時間です。

作成済みノード数を $$N$$、指定した版の数を $$Q$$ とすると、`front_many` は $$O(N+Q)$$ 時間・追加空間です。
