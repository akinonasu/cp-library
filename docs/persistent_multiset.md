---
title: 永続順序付き多重集合
documentation_of: //data_structure/persistent_multiset.hpp
redirect_from:
- /data_structure/persistent/persistent_multiset.hpp.html
- /lightweight/data_structure/persistent/persistent_multiset.hpp.html
library_category: data_structure/persistent
---

重複を数えながら、過去版ごとの挿入・削除・順位・k 番目・前後の値を求めます。通常の集合として使う場合は、`contains(root,x)` が偽のときだけ挿入します。

```cpp
#include "lib/data_structure/persistent_multiset.hpp"
#include <cassert>

int main() {
	PersistentMultiset<int> set;
	auto v0 = set.initial_root();
	auto v1 = set.insert(v0, 5, 3);
	auto v2 = set.insert(v1, 2);
	auto v3 = set.erase(v2, 5, 2);
	assert(set.size(v2) == 4 && set.count(v3, 5) == 1);
	assert(set.kth(v2, 0) == 2 && set.kth(v2, 3) == 5);
	assert(set.order_of_key(v2, 5) == 1);
	assert(set.lower_bound(v3, 3) == 5);
}
```

保存した各状態を版と呼び、その状態を指定する `root_type` の値をルートと呼びます。各更新は新しい版のルートを返し、引数で指定した版は変更しません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `PersistentMultiset<T,Compare>(compare = Compare{})` | 空集合。既定は `std::less<T>` | $$O(1)$$ |
| `build(a)` | `vector<T>` の全要素から版を作る。初期版は引き続き空集合 | $$O(n\log n)$$ |
| `size(root)` / `empty(root)` | 重複込みの要素数／空か | $$O(1)$$ |
| `insert(root,x,count = 1)` | `x` を `count` 個追加した版 | $$O(\log d)$$ |
| `erase(root,x,count = 1)` | `x` を最大 `count` 個削除した版。不足分は無視 | $$O(\log d)$$ |
| `erase_all(root,x)` | `x` を全て削除した版 | $$O(\log d)$$ |
| `count(root,x)` / `contains(root,x)` | `x` の個数／存在するか | $$O(\log d)$$ |
| `kth(root,k)` | 比較順で 0 始まり `k` 番目。重複込み | $$O(\log d)$$ |
| `order_of_key(root,x)` | 比較順で `x` より前にある要素数 | $$O(\log d)$$ |
| `lower_bound(root,x)` | `x` 以上の最初の値。なければ `nullopt` | $$O(\log d)$$ |
| `upper_bound(root,x)` | `x` より大きい最初の値。なければ `nullopt` | $$O(\log d)$$ |

`d` は異なる値の個数です。個数・順位は `std::int64_t` で、`count >= 0`、全体の個数は `INT64_MAX` 以下とします。`kth` は `0 <= k < size(root)` が必要です。`Compare(a,b)` は `a` が `b` より前に来るかを返します。比較では、自分自身には `false` を返し、`a` が `b` より前、`b` が `c` より前なら `a` が `c` より前となるようにします。どちら向きの比較も `false` のときは同値とみなし、`a` と `b`、`b` と `c` が同値なら `a` と `c` も同値になるようにします。同値な値は同じ値として数えます。`lower_bound`・`upper_bound` の大小も比較器に従います。`T` はコピー・代入可能な型とし、デフォルト構築は不要です。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。時間には領域を確保し直す費用も含め、一連の更新の合計から 1 回あたりの上限を求めています（償却計算量）。

一括構築は入力要素数を $$n$$ として $$O(n\log n)$$ 時間、追加ノード数は異なる値の個数 $$d$$ 個です。挿入・削除・検索・順位取得は $$O(\log d)$$ 時間、更新ごとの追加領域は $$O(\log d)$$ です。更新では個数に比例する繰り返しは行いません。空集合・異なる値が 1 個の場合は定数時間です。読み取りはノードを作りません。
