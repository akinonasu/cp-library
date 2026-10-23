---
title: Wavelet Matrix
documentation_of: //data_structure/wavelet_matrix/wavelet_matrix.hpp
redirect_from:
- /data_structure/wavelet_matrix.hpp.html
---

```cpp
#include "lib/data_structure/wavelet_matrix/wavelet_matrix.hpp"

WaveletMatrix<int> wm(std::vector<int>{3, 1, 4, 1, 5});
auto value = wm.kth_smallest(0, 5, 2); // 3
auto count = wm.count(0, 5, 1);        // 2
auto position = wm.select(1, 1);      // 3
auto total = wm.sum_smallest(0, 5, 3); // 5: 1 + 1 + 3
```

区間は $$[l,r)$$、添字と `kth_*`・`select` の $$k$$ は 0 始まりです。

一点取得は `0 <= i < size()`、区間は `0 <= l <= r <= size()` とします。`kth_*` は `0 <= k < r-l`、`sum_smallest`・`sum_largest` は `0 <= k <= r-l` が必要です。和の `k` は順位ではなく個数で、0 個なら 0 を返します。重複する値は個数分だけ数えます。

`count`・`rank`・`range_freq` と和は空区間で 0、`prev_value`・`next_value` は空区間で `nullopt` を返します。`select(x,k)` は元の列を左から数えた `k` 番目の `x` の位置で、値がない場合や順位が負・範囲外の場合は -1 です。値域の `lower >= upper` は空として扱います。

対応する CPU では AVX2・AVX-512 を自動的に利用します。追加の指定は不要です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WaveletMatrix()` / `WaveletMatrix(a)` | 空／配列から構築 | 空: $$O(1)$$、整数列: $$O(n\log\sigma)$$、その他: $$O(n\log n)$$ |
| `WaveletMatrix(a,true)` | 和の前処理も済ませて構築 | 整数列: $$O(n\log\sigma)$$、その他: $$O(n\log n)$$ |
| `build(a)` | 配列で再構築。和の前処理は必要時に行う | 整数列: $$O(n\log\sigma)$$、その他: $$O(n\log n)$$ |
| `size()` / `empty()` | 要素数／空判定 | $$O(1)$$ |
| `access(i)` / `operator[](i)` | 元の配列の要素を取得 | $$O(1)$$ |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ にある `x` の個数 | $$O(\log\sigma)$$、索引あり: 期待 $$O(\log\sigma)$$ |
| `prepare_count()` | 整数型の `count`・`rank`・`select` 用に値検索を前処理。完了済みなら何もしない | 期待 $$O(\sigma)$$、最悪 $$O(\sigma^2)$$ |
| `select(x,k)` | 0 始まり `k` 番目の `x` の位置。なければ -1 | $$O(\log\sigma)$$、索引あり: 期待 $$O(1)$$ |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 小さい順／大きい順の 0 始まり `k` 番目 | $$O(\log\sigma)$$ |
| `range_freq(l,r,upper)` | `upper` 未満の個数 | $$O(\log\sigma)$$ |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の個数 | $$O(\log\sigma)$$ |
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | `upper` 未満の最大値／`lower` 以上の最小値。なければ `nullopt` | $$O(\log\sigma)$$ |
| `prepare_sum()` | 和の前処理。完了済みなら何もしない | $$O(n\log\sigma)$$、準備済み: $$O(1)$$ |
| `range_sum(l,r,upper)` | `upper` 未満の値の和 | 準備後 $$O(\log\sigma)$$ |
| `range_sum(l,r,lower,upper)` | `lower` 以上 `upper` 未満の値の和 | 準備後 $$O(\log\sigma)$$ |
| `range_sum_with_count(l,r,upper)` | `upper` 未満の値の和と個数を `pair<Sum,int>` で返す | 準備後 $$O(\log\sigma)$$ |
| `range_sum(l,r)` | 区間和。未前処理なら初回に前処理 | 準備後 $$O(1)$$ |
| `sum_smallest(l,r,k)` / `sum_largest(l,r,k)` | 下位／上位 `k` 個の和。未前処理なら初回に前処理 | 準備後 $$O(\log\sigma)$$ |
| `min_count_smallest(l,r,target)` / `min_count_largest(l,r,target)` | 小さい順／大きい順に加え、和が `target` 以上になる最小個数。届かなければ -1 | 準備後 $$O(\log\sigma)$$、単調配列: $$O(\log n)$$ |
| `max_count_smallest(l,r,budget)` / `max_count_largest(l,r,budget)` | 小さい順／大きい順に加え、和が `budget` 以下の最大個数。空の選択も不可能なら -1 | 準備後 $$O(\log\sigma)$$、単調配列: $$O(\log n)$$ |

`range_sum_with_count` は和と個数を一度の走査で求めます。値域を指定する和も、未前処理なら最初に和の準備を行います。

和の前処理は必要時に行います。構築時に済ませる場合は `WaveletMatrix<T> wm(a, true)` とします。整数の和は 32 bit 型なら `std::int64_t`、64 bit 型なら `__int128_t` です。`WaveletMatrix<T, Sum>` で和の型を指定できます。

入力は `vector<T>`、長さは `INT_MAX` 以下です。`T` はコピー・代入と、互いに整合する `<`・`==`・`!=` に対応する型とし、浮動小数点の NaN は渡さないでください。入力は変更せず、構築後に破棄しても構いません。和を利用する場合は `Sum(0)`・`T` からの変換・加減算・整数との乗算が必要で、中間の部分和も `Sum` に収めてください。和を使わない場合は文字列などの値も扱えます。

通常のクエリは 8 分岐の索引を使います。AVX2・POPCNT に対応する x86 CPU では実行時に高速な経路を選び、それ以外ではスカラー処理を使います。和の前処理後は 4 分岐の索引を共用します。和を使うことが決まっている場合は構築時に `true` を指定すると、索引の作り直しを省けます。

`count`・`rank`・`select` を繰り返す場合は、`wm.prepare_count()` で値検索用のハッシュ表を準備できます。64 bit 以下の整数型に対応し、値の種類が少ない場合や連続している場合は追加の表を作りません。その他の型では何もしません。`build` で前処理は破棄されます。

`min_count_*`・`max_count_*` は非負整数列と整数の和の型で使います。`min_count_*` は `target <= 0` なら 0、`max_count_*` は `budget < 0` なら -1 を返します。和の準備は自動で行います。`sum_largest` などを個数について二分探索するより、索引を直接たどることで探索を一度に済ませます。

一点代入と和の探索には [Point Set Wavelet Matrix (クエリ先読み)](https://qandori.github.io/cp-library/data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp.html)、固定した値の重み更新には [Weighted Wavelet Matrix](https://qandori.github.io/cp-library/data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp.html) を使えます。

要素数を $$n$$、異なる値の数を $$\sigma$$ として、整数型の構築は $$O(n \log \sigma)$$、その他の型は $$O(n \log n)$$ 時間です。`access` は $$O(1)$$、`select` は $$O(\log \sigma)$$、その他のクエリは $$O(\log \sigma)$$ です。

和の前処理は時間・追加空間ともに $$O(n \log \sigma)$$ で、前処理後の `range_sum(l,r)` は $$O(1)$$、値域を指定する和と上下位の和は $$O(\log \sigma)$$ です。和を使う最初のクエリには前処理時間が加わります。全体の空間は和を含めて $$O(n \log \sigma)$$ です。比較・算術演算・コピーは $$O(1)$$ とします。異なる値が 1 種類以下なら構築・和の準備は $$O(n)$$、各クエリは $$O(1)$$ です。

`prepare_count` は期待 $$O(\sigma)$$・最悪 $$O(\sigma^2)$$ 時間、$$O(\sigma)$$ 空間です。前処理後の値検索は期待 $$O(1)$$・最悪 $$O(\sigma)$$、出現位置の検索を含む `count`・`rank` は期待 $$O(\log \sigma)$$、`select` は期待 $$O(1)$$ です。

和の準備後の `min_count_*`・`max_count_*` は通常 $$O(\log \sigma)$$、配列全体が昇順・降順の場合は $$O(\log n)$$ です。
