---
title: "Range Chmin Chmax Add Sum の使い方一覧"
redirect_from:
- /docs/segtree_beats_examples.html
---

[Range Chmin Chmax Add Sum](https://qandori.github.io/cp-library/data_structure/segment_tree/range_chmin_chmax_add_sum.hpp.html) は、次の更新と取得を任意に組み合わせて使えます。区間はすべて $$[l,r)$$ です。

除算や bitwise AND/OR など、別の更新・集約値を定義する場合は [汎用版](https://qandori.github.io/cp-library/data_structure/segment_tree/segtree_beats.hpp.html) を使ってください。

```cpp
#include "lib/data_structure/segment_tree/range_chmin_chmax_add_sum.hpp"

RangeChminChmaxAddSum seg(std::vector<long long>{1, 4, 2, 8});
```

以下はこの `seg` に対する操作例です。`l,r` は `0 <= l <= r <= seg.size()`、`p` は `0 <= p < seg.size()` とし、`x` は更新する値です。

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
