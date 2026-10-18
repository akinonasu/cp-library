---
title: "Interval Set"
documentation_of: //data_structure/interval_set.hpp
---

```cpp
#include "lib/data_structure/interval_set.hpp"

IntervalSet<> s;
s.insert(2, 7);
s.insert(7, 10);
s.erase(4, 6);
auto size = s.size();       // 6
auto mex = s.mex(2);        // 4
auto next = s.next(5);      // 6
for (auto [l, r] : s) {}    // [2,4), [6,10)
```

整数の集合を、重ならず隣接もしない半開区間 $$[l,r)$$ に圧縮します。隣接する区間は自動で結合します。座標型 `T` は `bool` を除く 64 bit 以下の整数型、既定は `std::int64_t` です。要素数は符号なし 64 bit の `size_type` で返します。

区間の両端は `T` で表せる値で、$$l\le r$$ が必要です。空区間の更新は何もしません。右端も `T` で表すため、格納できる整数は `T` の最小値以上、最大値未満です。`insert(x)` は `x` が `T` の最大値未満の場合に使ってください。最大値を右端・検索座標に使うことはでき、`erase(x)` は最大値なら `0` を返します。負の座標や、符号付き 64 bit の両端にまたがる区間にも対応します。

`next`・`prev` は検索座標自身を含み、該当する要素がなければ `std::nullopt` を返します。`mex(x)` は $$x$$ 以上で集合に含まれない最小の整数です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `IntervalSet()` | 空集合を作成 | $$O(1)$$ |
| `IntervalSet(intervals)` | `vector<pair<T,T>>` または初期化子リストの区間の和集合で初期化 | $$O(m\log m+n)$$ |
| `IntervalSet(s)` / `s = t` | コピー。区間も複製 | $$O(n)$$ |
| `IntervalSet(std::move(s))` / `s = std::move(t)` | 移動。移動元は空集合になる | 構築: $$O(1)$$、代入: 移動先の区間数に比例 |
| `assign(intervals)` | `vector<pair<T,T>>` の区間の和集合で置き換え。未ソート・重複・隣接を許す | $$O(m\log m+n)$$ |
| `insert(l,r)` / `insert(x)` | 区間／1 要素を追加。新しく増えた要素数を返す | 区間: $$O(\log n+k)$$、単点: $$O(\log n)$$ |
| `erase(l,r)` / `erase(x)` | 区間／1 要素を削除。実際に減った要素数を返す | 区間: $$O(\log n+k)$$、単点: $$O(\log n)$$ |
| `contains(x)` | 要素が含まれるか | $$O(\log n)$$ |
| `contains(l,r)` | 区間全体が含まれるか。空区間は `true` | $$O(\log n)$$ |
| `intersects(l,r)` | 区間に含まれる要素があるか。空区間は `false` | $$O(\log n)$$ |
| `count(l,r)` | 区間に含まれる要素数 | $$O(\log n+k)$$ |
| `find(x)` | `x` を含む区間のイテレータ。なければ `end()` | $$O(\log n)$$ |
| `lower_bound(x)` | `x` を含む区間、なければ `x` より右にある最初の区間のイテレータ。なければ `end()` | $$O(\log n)$$ |
| `next(x)` / `prev(x)` | `x` 以上の最小要素／`x` 以下の最大要素 | $$O(\log n)$$ |
| `mex(x = 0)` | `x` 以上の最小の非所属整数 | $$O(\log n)$$ |
| `min()` / `max()` | 最小要素／最大要素。空集合は `nullopt` | $$O(1)$$ |
| `size()` | 集合の要素数 | $$O(1)$$ |
| `interval_count()` | 圧縮後の区間数 | $$O(1)$$ |
| `empty()` | 空集合か | $$O(1)$$ |
| `begin()` / `end()` | 区間を左端の昇順に走査する読み取り専用イテレータ | $$O(1)$$ |
| `rbegin()` / `rend()` | 区間を降順に走査する読み取り専用イテレータ | $$O(1)$$ |
| `clear()` | 空集合に戻す | $$O(n)$$ |
| `swap(t)` / `swap(s,t)` | 集合を交換 | $$O(1)$$ |

`coordinate_type` は `T`、`size_type` は `std::uint64_t`、`interval_type` は `std::pair<T,T>` です。`const_iterator`・`const_reverse_iterator` の参照先は左端・右端の組です。更新時に結合・分割・削除・左端の変更を受けた区間のイテレータは使い直してください。構築にはソート後の挿入ヒント、更新には木のノードの再利用を使います。

操作前後の区間数の最大値を $$n$$、更新・列挙で触れる区間数を $$k$$ として、`insert`・`erase`・`count` は $$O(\log n+k)$$、単点更新・検索・`contains`・`intersects`・`find`・`lower_bound`・`next`・`prev`・`mex` は $$O(\log n)$$ です。長さ $$m$$ の区間列からの構築・`assign` は $$O(m\log m+n)$$、コピー・`clear` は $$O(n)$$ です。移動代入は移動先にあった区間数に比例します。それ以外の操作は $$O(1)$$、空間は $$O(n)$$、構築・`assign` 中は入力を含めて $$O(m+n)$$ です。
