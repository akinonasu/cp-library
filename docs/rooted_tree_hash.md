---
title: 根付き木のハッシュ
documentation_of: //tree/rooted_tree_hash.hpp
library_category: tree
---

無向木を指定した頂点で根付きにし、各頂点を根とする部分木の形のハッシュを求めます。頂点番号と隣接リストの順序に依存しません。各頂点に整数のラベルを付けた木にも対応します。ラベルは頂点に付けた値で、対応する頂点の値も等しい場合に同じ木とします。辺の値や重みは比較しません。

```cpp
#include "lib/tree/rooted_tree_hash.hpp"

std::vector<std::vector<int>> graph{ {1}, {0, 2, 3}, {1}, {1} };
RootedTreeHash tree(graph, 1);
bool same = tree[0] == tree[2]; // true: どちらも葉
auto whole = tree.get();       // 頂点 1 を根とする木全体
auto subtree = tree.get(2);    // tree.hash[2] と同じ
int parent = tree.parent[2];   // 1
int height = tree.height[1];   // 1
int size = tree.subtree_size[1]; // 4
```

頂点数を $$n$$ とします。

| 操作・フィールド | 内容・返り値 | 時間計算量 |
| --- | --- | --- |
| `RootedTreeHash()` | 空の状態を構築 | $$O(1)$$ |
| `RootedTreeHash(graph,root = 0,seed = default_seed())` | 根付き木と全ての部分木のハッシュを構築 | $$O(n)$$ |
| `RootedTreeHash(graph,labels,root = 0,seed = default_seed())` | 頂点ラベル付きの根付き木と全ての部分木のハッシュを構築 | $$O(n)$$ |
| `build(graph,root = 0,seed = default_seed())` | 指定した木で構築し直す | $$O(n)$$ |
| `build(graph,labels,root = 0,seed = default_seed())` | 頂点ラベル付きの木で構築し直す | $$O(n)$$ |
| `build_parents(parents,seed = default_seed())` | 親配列から構築し直す | $$O(n)$$ |
| `build_parents(parents,labels,seed = default_seed())` | 親配列と頂点ラベルから構築し直す | $$O(n)$$ |
| `classify()` | 同じハッシュを持つ部分木に同じ ID を付け、頂点順の `vector<int>` で返す | 期待 $$O(n)$$ |
| `get()` | 指定した根での木全体の `TreeHashValue`。空なら `TreeHashValue{}` | $$O(1)$$ |
| `get(v)` / `operator[](v)` / `hash[v]` | 頂点 `v` 以下の部分木の `TreeHashValue` | $$O(1)$$ |
| `root` | 根の頂点番号。空なら `-1` | $$O(1)$$ |
| `parent[v]` | 親。根では `-1` | $$O(1)$$ |
| `height[v]` | 部分木の高さ。葉では `0` | $$O(1)$$ |
| `subtree_size[v]` | 部分木の頂点数。自身も含む | $$O(1)$$ |
| `size()` / `empty()` | 頂点数の `int`／空かどうかの `bool` | $$O(1)$$ |
| `seed()` | 構築に使った seed を `std::uint64_t` で返す | $$O(1)$$ |
| `RootedTreeHash::default_seed()` | プロセス内で共通の既定 seed | $$O(1)$$ |


`graph` は `std::vector<std::vector<int>>` の無向木です。頂点数は `INT_MAX` 以下とし、各辺を両方向に 1 回ずつ入れ、連結・自己ループなし・多重辺なしとしてください。非空なら `0 <= root < size()` が必要です。空の木では根の引数は `0` または `-1` とし、結果の各配列は空になります。既定構築も空の状態です。

`build_parents` の親配列は長さが頂点数で、根の親だけを `-1` とし、親子関係が 1 本の根付き木になるようにしてください。頂点番号の大小に制約はありません。空配列にも対応します。

```cpp
#include "lib/tree/rooted_tree_hash.hpp"

RootedTreeHash tree;
tree.build_parents(std::vector<int>{-1, 0, 0});
auto ids = tree.classify(); // {0, 1, 1}
```

頂点ラベルは、長さ $$n$$ の `labels` を追加して指定します。`labels[v]` が頂点 `v` の値です。`vector`・固定長／可変長の `span`・`array`・`string` など、定数時間の `size()` と添字アクセスが使える列に対応します。要素型は `bool` を含む 64 bit 以下の標準整数型としてください。負数・整数型の最小値と最大値も区別します。別々の木を比較する場合も、同じ要素型の同じ値をそのまま使えます。ラベル列は空の木では空にし、構築後は保持する必要がありません。

```cpp
#include "lib/tree/rooted_tree_hash.hpp"
#include <cassert>

std::vector<std::vector<int>> graph{ {1, 2}, {0}, {0} };
std::vector<long long> labels{1000, -7, -7};
RootedTreeHash tree(graph, labels, 0, 12345);
assert(tree[1] == tree[2]); // 値も形も同じ葉

labels[2] = 257;
RootedTreeHash other;
other.build_parents(std::vector<int>{-1, 0, 0}, labels, 12345);
assert(tree.get() != other.get());
```

`classify()` の ID は 0 始まりで、同じハッシュなら同じ ID になります。新しい ID は頂点番号の小さい順に割り当てます。空の木では空列です。

構築結果は入力を参照せず、入力を変更しません。コピーして独立に保持できます。公開フィールドは構築結果を参照するためのものです。

`TreeHashValue` は `std::array<std::uint64_t,2> value` を持ち、2 本の 61 bit ハッシュをまとめて比較します。`==`・`!=`・大小比較に対応し、`std::map` のキーやソートに使えます。`std::unordered_map` では `TreeHashValue::Hasher` を指定します。

次の例は、直前のラベル付き木の例の続きです。

```cpp
#include <unordered_map>

std::unordered_map<TreeHashValue, int, TreeHashValue::Hasher> ids;
ids.emplace(tree[0], 0);
```

同じ seed を使っていれば、別々の木の部分木同士も比較できます。頂点ラベル付きではラベルの要素型も揃えてください。ラベル付きの結果とラベル無しの結果は比較しないでください。既定 seed は全ての木で共通です。実行をまたいで同じ値を得たい場合は、明示的に同じ seed を指定してください。`build` の seed を省略すると既定 seed に戻ります。

次の例は、最初の例で定義した `graph` を使います。

```cpp
RootedTreeHash first(graph, 0, 12345);
RootedTreeHash second(graph, 0, 12345);
bool same_tree = first.get() == second.get(); // true
```

根を対応させた上で頂点番号を付け替え、辺の接続関係と、ラベル付きなら各頂点の値が一致するか（根付き木の同型）を調べるためのハッシュです。子の並び順は区別しません。同型なら必ず一致しますが、異なる形が衝突する可能性はあります。2 本は高さごとに別々の乱数を使いますが、暗号学的な保証や衝突のない分類を提供するものではありません。各部分木で根を変えた場合の値は保持しません。根を無視して木全体を比較する場合は [木のハッシュ](tree_hash.md) を使ってください。

高さごとの乱数を用意し、各成分について次の積を子から親へ計算します。ここで $$h(v)$$ は頂点 $$v$$ の部分木の高さ、$$R_k[h]$$ は高さ $$h$$ ごとの乱数、$$k=0,1$$ は 2 本のハッシュの番号です。ラベル無しでは、空積は 1 なので葉の値は 2 本とも 1 です。

$$H_k(v)=\displaystyle\prod_{u\text{ は }v\text{ の子}}\bigl(R_k[h(v)]+H_k(u)\bigr)\pmod{2^{61}-1}.$$

ラベル付きでは、値 $$x$$ と seed から求めた初期値を $$C_k(x)$$ として、上の積に $$C_k(\mathrm{labels}[v])$$ を掛けます。葉の値もラベルによって決まります。

走査は再帰を使いません。子のソートや、同じ実行中に処理した他の木に依存する ID の割り当ても行いません。

頂点数を $$n$$ として、ラベルの有無によらず、構築・`build`・`build_parents` は $$O(n)$$ 時間・空間、`classify()` は期待 $$O(n)$$ 時間・$$O(n)$$ 追加空間です。ハッシュの取得・比較、各配列の参照は $$O(1)$$ 時間・追加空間です。固定幅整数の演算は定数時間として扱い、ラベル用の共有表や前計算は保持しません。
