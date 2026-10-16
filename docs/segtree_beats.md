---
title: "Segtree Beats"
documentation_of: //data_structure/segtree_beats.hpp
---

```cpp
#include "lib/data_structure/segtree_beats.hpp"

SegtreeBeats seg(std::vector<long long>{1, 4, 2, 8});
seg.chmin(0, 4, 5);
seg.chmax(1, 3, 3);
seg.add(0, 2, 10);
seg.assign(2, 4, 7);
auto s = seg.sum(0, 4);
auto mn = seg.min(0, 4);
auto mx = seg.max(0, 4);
```

[基本的な使い方の一覧](https://qandori.github.io/cp-library/docs/segtree_beats_examples.html)

区間は $$[l,r)$$ です。`SegtreeBeats<T = long long, Sum = T>` は符号付き整数を扱います。`SegtreeBeats<T, Sum>(n, x = 0)` は長さ `n` を `x` で初期化します。和だけ広い型にする場合は `SegtreeBeats<long long, __int128_t>` などを指定します。

| 操作 | 内容 |
| --- | --- |
| `chmin(l,r,x)` | 各要素を $$\min(a_i,x)$$ に更新 |
| `chmax(l,r,x)` | 各要素を $$\max(a_i,x)$$ に更新 |
| `add(l,r,x)` | 各要素に `x` を加算 |
| `assign(l,r,x)` | 各要素に `x` を代入 |
| `sum(l,r)`・`min(l,r)`・`max(l,r)` | 区間和・最小値・最大値 |
| `all_sum()`・`all_min()`・`all_max()` | 全体の和・最小値・最大値 |
| `get(p)`・`set(p,x)` | 1 点の取得・代入 |
| `values()` | 全要素を `vector<T>` として取得 |
| `size()` | 要素数 |

値は `numeric_limits<T>::lowest()` より大きく、`numeric_limits<T>::max()` より小さくしてください。値・累積加算は `T`、和・更新差分と個数の積は `Sum` に収まる必要があります。空区間の和は `0`、最小値・最大値はそれぞれ `numeric_limits<T>::max()`・`lowest()` です。

構築・`values()`・空間は $$O(n)$$、`add`・`assign`・区間取得・`get`・`set` は $$O(\log n)$$、`chmin`・`chmax` を含む更新は償却 $$O(\log^2 n)$$、全体の取得・`size()` は $$O(1)$$ です。
