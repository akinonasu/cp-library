---
title: Cartesian Tree
documentation_of: //tree/cartesian_tree.hpp
---

数列の添字を頂点とする二分木を構築します。中間順走査すると元の添字順に並び、親の値が子の値以下になります。既定では最小値が根になり、同じ値では左側の添字を優先します。

```cpp
#include "lib/tree/cartesian_tree.hpp"

std::vector<int> a{3, 1, 4, 1, 5};
auto tree = cartesian_tree(a);
int root = tree.root;          // 1
auto parent = tree.parent;     // {1, -1, 3, 1, 3}
auto left = tree.left;         // {-1, 0, -1, 2, -1}
auto right = tree.right;       // {-1, 3, -1, 4, -1}
auto maximum = cartesian_tree(a, std::greater<>{}); // 最大値が根
auto rightmost = cartesian_tree<true>(a);           // 同じ値なら右側優先
```

| 操作・フィールド | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `cartesian_tree<Rightmost = false>(a,compare = std::less<>{})` | 構築結果を `CartesianTree` で返す | $$O(n)$$ |
| `root` | 根の添字。空列では `-1` | $$O(1)$$ |
| `parent[v]` | 頂点 `v` の親。根では `-1` | $$O(1)$$ |
| `left[v]` / `right[v]` | 左の子／右の子。存在しなければ `-1` | $$O(1)$$ |
| `size()` / `empty()` | 頂点数／空かどうか | $$O(1)$$ |

入力は const なランダムアクセス・サイズ取得可能な range で、`vector`・`array`・`span`・`deque`・C 配列などを渡せます。入力を変更せず、要素をコピーしません。`vector<bool>` や、比較関数を指定したコピー不能な要素にも対応します。構築後の結果は入力を参照しません。頂点数は `INT_MAX` 以下としてください。

`compare(x,y)` が真なら `x` を `y` より親側に優先します。比較関数は厳密弱順序を定めるものを使います。`std::greater<>{}` を指定すると最大値を根にする木になります。

同じ値とは、比較関数でどちらも優先されないことを意味します。`Rightmost = false` では左側、`true` では右側を優先します。この規則は根だけでなく各部分木にも適用します。比較と添字による優先順位を合わせることで、重複を含む列でも木が一意に決まります。

公開配列は構築結果を参照するためのものです。

各要素を右端に加え、優先順位が逆転する右の祖先をたどってつなぎ替えます。親配列を単調スタックとして兼用し、再帰・番兵値・要素型ごとの固定バッファは使いません。各頂点は高々 1 回だけ右の経路から取り除かれます。

頂点数を $$n$$ として、構築は $$O(n)$$ 時間、返す 3 配列は $$O(n)$$ 空間です。出力以外の追加空間は $$O(1)$$ です。比較と入力へのランダムアクセスを $$O(1)$$ とします。
