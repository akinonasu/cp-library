---
title: Weighted Wavelet Matrix
documentation_of: //data_structure/weighted_wavelet_matrix.hpp
redirect_from:
- /data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp.html
- /lightweight/data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp.html
library_category: data_structure/wavelet_matrix
---

```cpp
#include "lib/data_structure/weighted_wavelet_matrix.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
	WeightedWaveletMatrix<int> wm(std::vector<int>{3, 1, 5},
	                             std::vector<std::int64_t>{2, 0, 1});
	wm.set_weight(1, 4);
	assert(wm.kth_smallest(0, 3, 2) == 1);
	assert(wm.min_count_largest(0, 3, 8) == 2);
	assert(wm.prev_value(0, 3, 3) == 1);
	assert(wm.next_value(0, 3, 3) == 3);
	assert(wm.rank(1, 3) == 4);
}
```

整数の値で個数や順位だけを問い合わせる場合は、`WeightedWaveletMatrix(a,w,false)` で和の前処理を省けます。後から和・上下位の和・和に対する個数探索を使う場合は、その前に非 const の `prepare_sum()` を呼びます。整数では既定で構築時に準備済みとなり、和の問い合わせ中には前処理や状態の変更を行いません。

値には文字列・`pair`・比較できる構造体なども使えます。その場合は個数・順位・重みの更新を利用でき、和の準備と和に関する操作は使えません。`with_sum` の既定値は `false` になり、明示的に指定する場合も `false` にしてください。

```cpp
#include "lib/data_structure/weighted_wavelet_matrix.hpp"
#include <cassert>
#include <string>
#include <vector>

int main() {
	std::vector<std::string> labels{"red", "blue", "red"};
	WeightedWaveletMatrix<std::string> wm(labels, {2, 1, 0});
	assert(wm.kth_smallest(0, 3, 0) == "blue");
	assert(wm.range_freq(0, 3, "red") == 1);
	wm.move_weight(0, 1, 1);
	assert(wm.count(0, 3, "blue") == 2);
}
```

値を固定し、各位置の非負整数の重みを更新します。重みは多重集合での個数を表し、重み 3 の値 `x` は `x` が 3 個あるものとして扱います。区間は $$[l,r)$$、順位は 0 始まりです。重みが 0 の位置も `size()` に含みます。

値と重みは同じ長さの `vector` を渡し、長さは `INT_MAX` 以下とします。一点操作は `0 <= i < size()`、区間は `0 <= l <= r <= size()` です。区間の個数 `c = count(l,r)` に対して、`kth_*` は `0 <= k < c`、`sum_smallest`・`sum_largest` は `0 <= k <= c` が必要です。空区間や重みの総和が 0 の区間の個数・和は 0、0 個を選んだ和も 0 です。同じ値の位置は重みを合計して数えます。入力は変更せず、構築後に破棄できます。

`prev_value`・`next_value` は重みが正の位置だけを対象にします。空区間や重みの総和が 0 の区間では `std::nullopt` を返します。`rank(x,r)` は `0 <= r <= size()` とし、返り値は重みと同じ整数型です。

固定した位置の数を $$n$$、異なる値の数を $$\sigma$$ とします。要素の比較・コピーを定数時間として、以下の計算量を表します。文字列などを使う場合は、実際の比較・コピーにかかる時間も含めて考えてください。異なる値が 0 個または 1 個の場合、対数の項は定数として扱います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WeightedWaveletMatrix()` | 空で構築 | $$O(1)$$ |
| `WeightedWaveletMatrix(a)` / `build(a)` | 全位置の重みを 1 として構築／再構築 | $$O(n\log\sigma)$$ |
| `WeightedWaveletMatrix(a,w,with_sum)` / `build(a,w,with_sum)` | 値と重みから構築／再構築。既定値は整数の値なら `true`、その他は `false` | $$O(n\log\sigma)$$ |
| `prepare_sum()` | 和の前処理。準備済みなら何もしない | 初回 $$O(n\log\sigma)$$、準備済み $$O(1)$$ |
| `size()` / `empty()` | 固定した位置の数／空判定 | $$O(1)$$ |
| `access(i)` / `operator[](i)` | 固定した値を取得 | $$O(1)$$ |
| `weight(i)` | 現在の重み | $$O(1)$$ |
| `set_weight(i,w)` / `add_weight(i,delta)` | 重みの代入／加算。更新後も非負とする | $$O(\log n\log\sigma)$$ |
| `move_weight(i,j,w)` | 位置 `i` から `j` に個数 `w` を移す。`w <= weight(i)` とする | $$O(\log n\log\sigma)$$ |
| `count(l,r)` | 区間の重みの総和 | $$O(\log n)$$ |
| `count(l,r,x)` / `rank(x,r)` | 区間／$$[0,r)$$ の値が `x` の位置の重みの総和 | $$O(\log n\log\sigma)$$ |
| `range_freq(l,r,upper)` | `upper` 未満の重みの総和 | $$O(\log n\log\sigma)$$ |
| `range_freq(l,r,lower,upper)` | `lower` 以上 `upper` 未満の重みの総和 | $$O(\log n\log\sigma)$$ |
| `range_sum(l,r)` | 区間の `値 * 重み` の総和 | $$O(\log n)$$ |
| `range_sum(l,r,upper)` | `upper` 未満の `値 * 重み` の総和 | $$O(\log n\log\sigma)$$ |
| `range_sum(l,r,lower,upper)` | 指定した値域の `値 * 重み` の総和 | $$O(\log n\log\sigma)$$ |
| `kth_smallest(l,r,k)` / `kth_largest(l,r,k)` | 重みを個数として、小さい順／大きい順の `k` 番目 | $$O(\log n\log\sigma)$$ |
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | `upper` 未満の最大値／`lower` 以上の最小値。返り値は `std::optional<T>`、該当なしは `std::nullopt` | $$O(\log n\log\sigma)$$ |
| `sum_smallest(l,r,k)` / `sum_largest(l,r,k)` | 下位／上位 `k` 個の和 | $$O(\log n\log\sigma)$$ |
| `min_count_smallest(l,r,target)` / `min_count_largest(l,r,target)` | 各順序で和が `target` 以上になる最小個数。届かなければ -1 | $$O(\log n\log\sigma)$$ |
| `max_count_smallest(l,r,budget)` / `max_count_largest(l,r,budget)` | 各順序で和が `budget` 以下の最大個数。空の選択も不可能なら -1 | $$O(\log n\log\sigma)$$ |

`T` はコピーでき、`<` で矛盾なく大小を判定できる型です。互いに `<` で小さくない値は同じキーとしてまとめます。`==` や既定コンストラクタは必要ありません。浮動小数点数の NaN は使わないでください。`bool` は使えません。`Count` は 64 bit 以下の符号付き整数型、`Sum` は符号付き整数型です。`WeightedWaveletMatrix<T,Count,Sum>` で型を指定できます。既定の `Count` は `std::int64_t`、`Sum` は `sizeof(T) < 8` なら `std::int64_t`、それ以外は `__int128_t` です。

比較で同じキーになる値を複数登録した場合、`access`・`kth_*` はそのキーの代表値を返します。構造体で比較に使わない情報を持つ場合、その情報まで元の位置の値と一致するとは限りません。

`prepare_sum`・`range_sum`・`sum_smallest`・`sum_largest`・`min_count_*`・`max_count_*` は、`T` が 64 bit 以下の標準整数型の場合だけ利用できます。個数の総和が `Count` に、更新を含む中間の部分和と値・重みの積が `Sum` に収まるようにします。重みが大きい場合は 32 bit の値でも `Sum` に `__int128_t` を指定してください。

`min_count_*`・`max_count_*` には登録した値全てが非負であることが必要です。前者は `target <= 0` なら 0、後者は `budget < 0` なら -1 を返します。これ以外の操作は負の値も扱えます。

値の代入が必要で候補を先読みできる場合は [Point Set Wavelet Matrix (クエリ先読み)](https://qandori.github.io/cp-library/data_structure/point_set_wavelet_matrix.hpp.html) を使えます。コピーと `build` による再構築も可能です。

構築・空間は $$O(n\log\sigma)$$、更新・値域を指定するクエリ・順位と和の探索は $$O(\log n\log\sigma)$$ です。`count(l,r)`・`range_sum(l,r)` は $$O(\log n)$$、サイズと要素・重みの取得は $$O(1)$$ です。要素自身が持つ文字列などの格納領域は別途必要です。
