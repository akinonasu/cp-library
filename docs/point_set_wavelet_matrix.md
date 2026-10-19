---
title: Point Set Wavelet Matrix
documentation_of: //data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp
redirect_from:
- /data_structure/point_set_wavelet_matrix.hpp.html
---

```cpp
#include "lib/data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp"

std::vector<int> a{3, 1, 4, 1, 5};
std::vector<std::pair<int, int>> candidates{ {0, 10}, {2, 7} };
PointSetWaveletMatrix<int> wm(a, candidates);
wm.set(0, 10);
auto answer = wm.min_count_largest(0, 5, 12);
```

配列の一点代入と、区間内の順位・和の問い合わせを扱います。初期値は自動登録されます。それ以外に代入する `(位置,値)` は `candidates` に登録します。問い合わせは先読みする必要がありません。区間は $$[l,r)$$、順位は 0 始まりです。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `PointSetWaveletMatrix()` | 空で構築 | $$O(1)$$ |
| `PointSetWaveletMatrix(a,candidates = {})` / `build(a,candidates = {})` | 初期配列と代入候補から構築／再構築 | $$O(m\log m)$$ |
| `size()` / `empty()` | 配列の長さ／空判定 | $$O(1)$$ |
| `access(i)` / `operator[](i)` | 現在の値 | $$O(1)$$ |
| `contains(i,x)` | 位置 `i` に値 `x` を代入できるか | $$O(\log m)$$ |
| `set(i,x)` | 一点代入。未登録なら false を返し、内容を保持 | $$O(\log m\log\sigma)$$ |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ の `x` の個数 | $$O(\log m\log\sigma)$$ |
| `range_freq(l,r,upper)` | `upper` 未満の個数 | $$O(\log m\log\sigma)$$ |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の個数 | $$O(\log m\log\sigma)$$ |
| `range_sum(l,r)` | 区間和 | $$O(\log m)$$ |
| `range_sum(l,r,upper)` | `upper` 未満の値の和 | $$O(\log m\log\sigma)$$ |
| `range_sum(l,r,lower,upper)` | 指定した値域の和 | $$O(\log m\log\sigma)$$ |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 小さい順／大きい順の `k` 番目 | $$O(\log m\log\sigma)$$ |
| `sum_smallest(l,r,k)` / `sum_largest(l,r,k)` | 下位／上位 `k` 個の和 | $$O(\log m\log\sigma)$$ |
| `min_count_smallest(l,r,target)` / `min_count_largest(l,r,target)` | 各順序で和が `target` 以上になる最小個数。届かなければ -1 | $$O(\log m\log\sigma)$$ |
| `max_count_smallest(l,r,budget)` / `max_count_largest(l,r,budget)` | 各順序で和が `budget` 以下の最大個数。空の選択も不可能なら -1 | $$O(\log m\log\sigma)$$ |

`T` は 64 bit 以下の整数型です。`PointSetWaveletMatrix<T,Sum>` で和の型を指定できます。既定は 32 bit 以下の値なら `std::int64_t`、64 bit の値なら `__int128_t` です。

和の境界探索 `min_count_*`・`max_count_*` には初期値と登録した候補全てが非負であることが必要です。前者は `target <= 0` なら 0、後者は `budget < 0` なら -1 を返します。その他の問い合わせは負の値も扱えます。

[ABC467 G](https://atcoder.jp/contests/abc467/tasks/abc467_g) は更新候補を登録した後、各クエリで `wm.set(c,x)` と `wm.min_count_largest(l,r,k)` を呼べば解けます。[提出用コード](https://github.com/qandori/cp-library/blob/main/test/benchmark/abc467_g.cpp)もあります。

異なる登録 `(位置,値)` の総数を $$m$$、異なる値の数を $$\sigma$$ として、構築は $$O(m\log m)$$ 時間、空間は $$O(m\log\sigma)$$ です。更新と区間の順位・和の探索は $$O(\log m\log\sigma)$$、値域を指定しない区間和は $$O(\log m)$$、候補の確認は $$O(\log m)$$、サイズと要素取得は $$O(1)$$ です。
