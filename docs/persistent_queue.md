---
title: 永続キュー
documentation_of: //data_structure/persistent_queue.hpp
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

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `size(root)` / `empty(root)` | 要素数／空か | $$O(1)$$ |
| `push(root,x)` | `x` を末尾に追加した版 | $$O(1)$$ |
| `pop(root)` | 先頭を削除した版。非空が必要 | $$O(1)$$ |
| `front(root)` | 先頭要素。非空が必要 | $$O(\log u)$$ |
| `back(root)` | 末尾要素。非空が必要 | $$O(1)$$ |
| `get(root,p)` | 先頭から 0 始まり `p` 番目の要素 | $$O(\log u)$$ |

`get` は `0 <= p < size(root)` とします。`u` は空になるまでの同じ分岐上での push 回数です。`u <= INT_MAX` とし、`T` はコピー可能な型とします。`T` のデフォルト構築は不要です。`root_type` のフィールドは書き換えず、そのまま保存してください。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新の時間には領域確保の償却計算量を含みます。

push は償却 $$O(1)$$、pop・back・size は $$O(1)$$、front・添字アクセスは $$O(\log u)$$ 時間です。push 1 回につき 1 ノードを追加します。他の操作はノードを作りません。過去版から何度分岐してもこれらの計算量を保ちます。空または 1 ノードの履歴に対する操作は定数時間です。
