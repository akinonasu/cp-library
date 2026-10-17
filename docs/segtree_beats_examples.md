---
title: "Segtree Beats の使い方一覧"
---

[Segtree Beats](https://qandori.github.io/cp-library/data_structure/segtree_beats.hpp.html) は、次の更新と取得を任意に組み合わせて使えます。区間はすべて $$[l,r)$$ です。

除算や bitwise AND/OR など、別の更新・集約値を定義する場合は [汎用版](https://qandori.github.io/cp-library/data_structure/generic_segtree_beats.hpp.html) を使ってください。

```cpp
#include "lib/data_structure/segtree_beats.hpp"

SegtreeBeats seg(std::vector<long long>{1, 4, 2, 8});
```

| 更新 ＼ 取得 | 区間和 | 区間最小値 | 区間最大値 |
| --- | --- | --- | --- |
| 加算 | `add` + `sum` | `add` + `min` | `add` + `max` |
| 代入 | `assign` + `sum` | `assign` + `min` | `assign` + `max` |
| 上限で切る | `chmin` + `sum` | `chmin` + `min` | `chmin` + `max` |
| 下限で切る | `chmax` + `sum` | `chmax` + `min` | `chmax` + `max` |

## 区間加算

```cpp
seg.add(l, r, x);
auto sum = seg.sum(l, r);
auto min = seg.min(l, r);
auto max = seg.max(l, r);
```

## 区間代入

```cpp
seg.assign(l, r, x);
auto sum = seg.sum(l, r);
auto min = seg.min(l, r);
auto max = seg.max(l, r);
```

## 区間 chmin

各要素を $$\min(a_i,x)$$ にします。

```cpp
seg.chmin(l, r, x);
auto sum = seg.sum(l, r);
auto min = seg.min(l, r);
auto max = seg.max(l, r);
```

## 区間 chmax

各要素を $$\max(a_i,x)$$ にします。

```cpp
seg.chmax(l, r, x);
auto sum = seg.sum(l, r);
auto min = seg.min(l, r);
auto max = seg.max(l, r);
```

## 値を上下限の範囲に収める

$$\mathrm{low}\leq\mathrm{high}$$ とします。

```cpp
seg.chmax(l, r, low);
seg.chmin(l, r, high);
```

## 更新を組み合わせる

```cpp
seg.add(l, r, x);
seg.chmin(l, r, upper);
seg.chmax(l, r, lower);
auto sum = seg.sum(l, r);
```

## 1 点・全体の取得

```cpp
seg.set(p, x);
auto value = seg.get(p);
auto sum = seg.all_sum();
auto min = seg.all_min();
auto max = seg.all_max();
auto a = seg.values();
```

構築・全要素取得・空間は $$O(n)$$、加算・代入・区間取得・1 点操作は $$O(\log n)$$、`chmin`・`chmax` を含む更新は償却 $$O(\log^2 n)$$、全体の和・最小・最大の取得は $$O(1)$$ です。
