---
title: 永続配列
documentation_of: //data_structure/persistent_array.hpp
redirect_from:
- /data_structure/persistent/persistent_array.hpp.html
- /lightweight/data_structure/persistent/persistent_array.hpp.html
library_category: data_structure/persistent
---

過去の配列を残したまま一点を更新します。区間積も必要なら [永続セグメント木](persistent_segtree.md) を使ってください。

```cpp
#include "lib/data_structure/persistent_array.hpp"
#include <cassert>

int main() {
	PersistentArray<int> a(100, -1);
	auto v0 = a.initial_root();
	auto v1 = a.set(v0, 3, 10);
	auto v2 = a.set(v0, 7, 20);
	assert(a.get(v0, 3) == -1);
	assert(a.get(v1, 3) == 10 && a.get(v1, 7) == -1);
	assert(a.get(v2, 3) == -1 && a.get(v2, 7) == 20);
}
```

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `PersistentArray<T>(n = 0, value = T{})` | 長さ `n`、全要素 `value` の初期版。疎に保持する | $$O(1)$$ |
| `PersistentArray<T>(a, default_value = T{})` | `vector<T>` から初期版を構築 | $$O(n)$$ |
| `get(root,p)` | 版 `root` の位置 `p` の値 | $$O(\log n)$$ |
| `set(root,p,x)` | 位置 `p` を `x` にした版 | $$O(\log n)$$ |
| `set_pair(root,p,x,q,y)` | `p` を `x`、`q` を `y` にした版。同じ位置なら `y` を使う | $$O(\log n)$$ |
| `size()` / `empty()` | 固定長／長さが 0 か | $$O(1)$$ |

`0 <= n <= INT_MAX`、`0 <= p < n` とします。`T` はコピー・代入可能な型です。デフォルト構築できない型は初期値と `default_value` を明示してください。`default_value` は内部の未作成領域の値で、配列から構築した初期版の値には影響しません。第 2 テンプレート引数 `Log`（既定値 3、範囲 1〜5）で分岐数を指定できます。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新の時間には領域確保の償却計算量を含みます。

構築は長さ指定なら $$O(1)$$、配列指定なら $$O(n)$$、取得・更新は $$O(\log n)$$ 時間です。更新ごとの追加領域は $$O(\log n)$$ です。長さ 0・1 の操作は定数時間です。
