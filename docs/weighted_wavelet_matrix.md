---
title: "Weighted Wavelet Matrix"
documentation_of: //data_structure/weighted_wavelet_matrix.hpp
---

```cpp
#include "lib/data_structure/weighted_wavelet_matrix.hpp"

WeightedWaveletMatrix<int> wm(std::vector<int>{3, 1, 5},
                            std::vector<std::int64_t>{2, 0, 1});
wm.set_weight(1, 4);
auto value = wm.kth_smallest(0, 3, 2);
auto count = wm.min_count_largest(0, 3, 8);
```

値を固定し、各位置の非負整数の重みを更新します。重みは多重集合での個数を表し、重み 3 の値 `x` は `x` が 3 個あるものとして扱います。区間は $$[l,r)$$、順位は 0 始まりです。重みが 0 の位置も `size()` に含みます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WeightedWaveletMatrix()` | 空で構築 | $$O(1)$$ |
| `WeightedWaveletMatrix(a)` / `build(a)` | 全位置の重みを 1 として構築／再構築 | $$O(n\log\sigma)$$ |
| `WeightedWaveletMatrix(a,w)` / `build(a,w)` | 値と重みから構築／再構築 | $$O(n\log\sigma)$$ |
| `size()` / `empty()` | 固定した位置の数／空判定 | $$O(1)$$ |
| `access(i)` / `operator[](i)` | 固定した値を取得 | $$O(1)$$ |
| `weight(i)` | 現在の重み | $$O(1)$$ |
| `set_weight(i,w)` / `add_weight(i,delta)` | 重みの代入／加算。更新後も非負とする | $$O(\log n\log\sigma)$$ |
| `move_weight(i,j,w)` | 位置 `i` から `j` に個数 `w` を移す。`w <= weight(i)` とする | $$O(\log n\log\sigma)$$ |
| `count(l,r)` | 区間の重みの総和 | $$O(\log n)$$ |
| `count(l,r,x)` | 値が `x` の位置の重みの総和 | $$O(\log n\log\sigma)$$ |
| `range_freq(l,r,upper)` | `upper` 未満の重みの総和 | $$O(\log n\log\sigma)$$ |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の重みの総和 | $$O(\log n\log\sigma)$$ |
| `range_sum(l,r)` | 区間の `値 * 重み` の総和 | $$O(\log n)$$ |
| `range_sum(l,r,upper)` | `upper` 未満の `値 * 重み` の総和 | $$O(\log n\log\sigma)$$ |
| `range_sum(l,r,lower,upper)` | 指定した値域の `値 * 重み` の総和 | $$O(\log n\log\sigma)$$ |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 重みを個数として、小さい順／大きい順の `k` 番目 | $$O(\log n\log\sigma)$$ |
| `sum_smallest(l,r,k)` / `sum_largest(l,r,k)` | 下位／上位 `k` 個の和 | $$O(\log n\log\sigma)$$ |
| `min_count_smallest(l,r,target)` / `min_count_largest(l,r,target)` | 各順序で和が `target` 以上になる最小個数。届かなければ -1 | $$O(\log n\log\sigma)$$ |
| `max_count_smallest(l,r,budget)` / `max_count_largest(l,r,budget)` | 各順序で和が `budget` 以下の最大個数。空の選択も不可能なら -1 | $$O(\log n\log\sigma)$$ |

`T` は 64 bit 以下の整数型、`Count` は符号付き整数型です。`WeightedWaveletMatrix<T,Count,Sum>` で型を指定できます。既定の `Count` は `std::int64_t`、`Sum` は 32 bit 以下の値なら `std::int64_t`、64 bit の値なら `__int128_t` です。個数の総和が `Count` に、更新を含む和の計算が `Sum` に収まるようにします。重みが大きい場合は 32 bit の値でも `Sum` に `__int128_t` を指定してください。

`min_count_*`・`max_count_*` には登録した値全てが非負であることが必要です。前者は `target <= 0` なら 0、後者は `budget < 0` なら -1 を返します。これ以外の操作は負の値も扱えます。

値の代入が必要で候補を先読みできる場合は [Point Set Wavelet Matrix](https://qandori.github.io/cp-library/data_structure/point_set_wavelet_matrix.hpp.html) を使えます。コピーと `build` による再構築も可能です。

固定した位置の数を $$n$$、異なる値の数を $$\sigma$$ として、構築・空間は $$O(n\log\sigma)$$、更新・値域を指定する問い合わせ・順位と和の探索は $$O(\log n\log\sigma)$$ です。`count(l,r)`・`range_sum(l,r)` は $$O(\log n)$$、サイズと要素・重みの取得は $$O(1)$$ です。
