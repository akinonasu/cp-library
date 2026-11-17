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

各バージョンは `root_type` の値（ルート）で指定します。各更新は新しいバージョンのルートを返し、引数で指定したバージョンは変更しません。`initial_root()` が最初のバージョンです。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去のバージョンから分岐できます。同じオブジェクトから得たルートだけを渡してください。バージョンごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `PersistentArray<T>(n = 0, value = T{})` | 長さ `n`、全要素 `value` の初期バージョン。初期値のままの部分にはノードを作らない | $$O(1)$$ |
| `PersistentArray<T>(a, default_value = T{})` | `vector<T>` から初期バージョンを構築 | $$O(n)$$ |
| `get(root,p)` | バージョン `root` の位置 `p` の値 | $$O(\log n)$$ |
| `set(root,p,x)` | 位置 `p` を `x` にしたバージョン | 償却 $$O(\log n)$$ |
| `set_pair(root,p,x,q,y)` | `p` を `x`、`q` を `y` にしたバージョン。同じ位置なら `y` を使う | 償却 $$O(\log n)$$ |
| `size()` / `empty()` | 固定長／長さが 0 か | $$O(1)$$ |

`0 <= n <= INT_MAX`、`0 <= p < n` とします。`T` はコピー・代入可能な型です。デフォルト構築できない型は初期値と `default_value` を明示してください。`default_value` は内部の未作成領域の値で、配列から構築した初期バージョンの値には影響しません。第 2 テンプレート引数 `Log`（既定値 3、範囲 1〜5）で分岐数を指定できます。

`reserve(count)` は全バージョンを合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新時間は領域確保を含む償却計算量です。

全バージョンを合わせた作成済みノード数を $$K$$、確保容量を $$C$$ とすると、保持空間は $$O(C)$$、オブジェクト自体のコピーは $$O(K)$$ 時間・追加空間です。コピー代入には、代入先の旧ノードを破棄する時間も加わります。`initial_root()`・`node_count()`・`memory_usage()` は $$O(1)$$、`reserve(count)` は容量が十分なら $$O(1)$$、再確保時は $$O(K)$$ 時間です。作成済みノード数と `count` は $$2^{32}-2$$ 以下とし、ノードがない場合にもコピー・予約には定数時間かかります。

構築は長さ指定なら $$O(1)$$、配列指定なら $$O(n)$$、取得・更新は $$O(\log n)$$ 時間です。更新ごとの追加領域は $$O(\log n)$$ です。長さ 0・1 の操作は定数時間です。
