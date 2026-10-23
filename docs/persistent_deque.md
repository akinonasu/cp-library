---
title: 永続 Deque・分割連結可能な列
documentation_of: //data_structure/persistent/persistent_deque.hpp
redirect_from:
  - /data_structure/persistent_deque.hpp.html
  - /lightweight/data_structure/persistent_deque.hpp.html
---

両端の追加・削除に加え、添字アクセス、一点代入、途中への挿入・削除、列の分割・連結を行います。同じ列を自分自身と連結することもできます。区間積には [永続セグメント木](persistent_segtree.md)、区間更新には [永続遅延セグメント木](persistent_lazy_segtree.md) を使います。

```cpp
#include "lib/data_structure/persistent/persistent_deque.hpp"
#include <cassert>

int main() {
	PersistentDeque<int> deque;
	auto v0 = deque.build({1, 2, 3});
	auto v1 = deque.push_front(v0, 0);
	auto v2 = deque.set(v0, 1, 9);
	auto [left, right] = deque.split(v1, 2);
	auto v3 = deque.concat(right, left);
	assert(deque.to_vector(v3) == (std::vector<int>{2, 3, 0, 1}));
	assert(deque.get(v0, 1) == 2 && deque.get(v2, 1) == 9);
}
```

各更新は新しい `root_type` を返し、引数の版は変わりません。`initial_root()` が最初の版です。ルートをコピーするだけなら時間・追加領域ともに $$O(1)$$ で、任意の過去版から分岐できます。同じオブジェクトから得たルートだけを渡してください。版ごとのノード解放は行いません。オブジェクト自体のコピーは全ノードの複製になります。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `build(a)` | `vector<T>` から版を作る。初期版は引き続き空列 | $$O(n)$$ |
| `size(root)` / `empty(root)` | 要素数／空か | $$O(1)$$ |
| `get(root,p)` | 位置 `p` の要素 | $$O(\log n)$$ |
| `front(root)` / `back(root)` | 先頭／末尾要素。非空が必要 | $$O(\log n)$$ |
| `push_front(root,x)` / `push_back(root,x)` | 先頭／末尾に追加した版 | $$O(\log n)$$ |
| `pop_front(root)` / `pop_back(root)` | 先頭／末尾を削除した版。非空が必要 | $$O(\log n)$$ |
| `set(root,p,x)` | 位置 `p` を置換した版 | $$O(\log n)$$ |
| `insert(root,p,x)` | 位置 `p` の直前に挿入した版。末尾なら `p = size(root)` | $$O(\log n)$$ |
| `erase(root,l,r)` | 半開区間 `[l,r)` を削除した版 | $$O(\log n)$$ |
| `split(root,p)` | `[0,p)` と `[p,n)` の 2 つのルート | $$O(\log n)$$ |
| `concat(left,right)` | 2 つの版をこの順に連結した版 | $$O(\log(n+m))$$ |
| `to_vector(root)` | 要素順に `vector<T>` に変換 | $$O(n)$$ |
| `node_count()` | 全版で作成したノード数 | $$O(1)$$ |

添字は 0 始まり、`get`・`set` は `0 <= p < n`、`split`・`insert` は `0 <= p <= n`、`erase` は `0 <= l <= r <= n` とします。連結後も長さは `INT_MAX` 以下にしてください。`T` はデフォルト構築・コピー・代入できる型です。

構築は $$O(n)$$、各更新・分割・添字アクセスは $$O(\log n)$$、連結は $$O(\log(n+m))$$ 時間です。各更新の追加領域は $$O(\log n)$$、連結では $$O(\log(n+m))$$ です。時間には領域確保の償却計算量を含みます。空列・長さ 1 の操作は定数時間です。読み取りはノードを作りません。
