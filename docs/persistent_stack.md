---
title: 永続スタック
documentation_of: //data_structure/persistent_stack.hpp
redirect_from:
- /data_structure/persistent/persistent_stack.hpp.html
- /lightweight/data_structure/persistent/persistent_stack.hpp.html
library_category: data_structure/persistent
---

過去版からの push・pop と、先頭から指定個数戻った要素の取得を行います。

```cpp
#include "lib/data_structure/persistent_stack.hpp"
#include <cassert>

int main() {
	PersistentStack<int> stack;
	auto v0 = stack.initial_root();
	auto v1 = stack.push(v0, 10);
	auto v2 = stack.push(v1, 20);
	auto v3 = stack.push(v1, 30);
	assert(stack.top(v2) == 20 && stack.top(v3) == 30);
	assert(stack.get(v2, 1) == 10 && stack.pop(v2) == v1);
}
```

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `size(root)` / `empty(root)` | 要素数／空か | $$O(1)$$ |
| `push(root,x)` | `x` を先頭に追加した版 | $$O(1)$$ |
| `pop(root)` | 先頭を削除した版。非空が必要 | $$O(1)$$ |
| `top(root)` | 先頭要素。非空が必要 | $$O(1)$$ |
| `get(root,k)` | 先頭から 0 始まり `k` 番目の要素 | $$O(\log n)$$ |
| `ancestor(root,k)` | `k` 個 pop した版。`0 <= k <= size(root)` | $$O(\log n)$$ |

`get` は `0 <= k < size(root)` とします。要素数は `INT_MAX` 以下、`T` はコピー可能な型とします。`T` のデフォルト構築は不要です。

`get_many(queries)` は `std::vector<std::pair<root_type,int>>` で渡した `(root,k)` それぞれの要素を、問い合わせ順の `std::vector<T>` として返します。各組は `get(root,k)` と同じ範囲を満たしてください。空の問い合わせ列には空の列を返します。問い合わせ数は `INT_MAX` 以下とします。ルートや格納済みのノードは変更しません。

`reserve(count)` は全版を合わせたノード数を予約します。`node_count()` は作成済みノード数、`memory_usage()` は予約済みノード領域のバイト数を返します。ルートは再確保後も有効です。値のコピー・演算は定数時間とします。更新の時間には領域確保の償却計算量を含みます。

push は償却 $$O(1)$$、pop・top・size は $$O(1)$$、祖先・添字アクセスは $$O(\log n)$$ 時間です。push 1 回につき 1 ノードを追加し、他の操作はノードを作りません。長さ 0・1 の操作は定数時間です。

作成済みノード数を $$N$$、問い合わせ数を $$Q$$ とすると、`get_many` は $$O(N+Q)$$ 時間・追加空間です。
