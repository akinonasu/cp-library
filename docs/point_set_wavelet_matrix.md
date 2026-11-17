---
title: Point Set Wavelet Matrix (クエリ先読み)
documentation_of: //data_structure/point_set_wavelet_matrix.hpp
redirect_from:
- /data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp.html
- /lightweight/data_structure/wavelet_matrix/point_set_wavelet_matrix.hpp.html
library_category: data_structure/wavelet_matrix
---

```cpp
#include "lib/data_structure/point_set_wavelet_matrix.hpp"
#include <cassert>
#include <utility>
#include <vector>

int main() {
	std::vector<int> a{3, 1, 4, 1, 5};
	std::vector<std::pair<int, int>> candidates{ {0, 10}, {2, 7} };
	PointSetWaveletMatrix<int> wm(a, candidates);
	assert(wm.set(0, 10));
	assert(wm.min_count_largest(0, 5, 12) == 2);
	assert(wm.prev_value(0, 5, 5) == 4);
	assert(wm.next_value(0, 5, 5) == 5);
}
```

配列の一点代入と、区間内の順位・和のクエリを扱います。初期値以外に代入する全ての `(位置,値)` の事前登録が必要です。通常は構築前に更新クエリを先読みし、候補を `candidates` に集めます。初期値は自動登録されます。候補が最初から分かっていれば、更新クエリ自体の先読みは不要です。順位・和を求めるクエリは先読みせずに処理できます。区間は $$[l,r)$$、順位は 0 始まりです。

値には文字列・`pair`・比較できる構造体なども使えます。その場合は個数・順位・一点代入を利用でき、和に関する操作は使えません。

```cpp
#include "lib/data_structure/point_set_wavelet_matrix.hpp"
#include <cassert>
#include <string>
#include <vector>

int main() {
	std::vector<std::string> labels{"red", "blue", "red"};
	std::vector<std::pair<int, std::string>> candidates{ {0, "green"}, {2, "blue"} };
	PointSetWaveletMatrix<std::string> wm(labels, candidates);
	assert(wm.set(0, "green"));
	assert(wm.kth_smallest(0, 3, 1) == "green");
	assert(wm.range_freq(0, 3, "red") == 2);
	assert(!wm.set(1, "unregistered"));
}
```

一点操作と候補の位置は `0 <= i < size()`、区間は `0 <= l <= r <= size()` とします。`kth_*` は `0 <= k < r-l`、`sum_smallest`・`sum_largest` は `0 <= k <= r-l` が必要です。和の `k` は個数で、0 個の和は 0 です。重複する要素は個数分だけ数え、空区間の個数・和は 0 です。同じ候補を複数登録しても一つにまとめます。列長と異なる登録 `(位置,値)` の総数はそれぞれ `INT_MAX` 以下とします。入力は変更せず、構築後に破棄できます。

`prev_value`・`next_value` は現在の配列を対象にし、代入候補として登録しただけの値は含めません。空区間では `std::nullopt` を返します。

初期配列の長さと登録候補の数の合計を $$m$$（重複候補も数えます）、異なる値の数を $$\sigma$$ とします。要素の比較・コピーを定数時間として、以下の計算量を表します。文字列などを使う場合は、実際の比較・コピーにかかる時間も含めて考えてください。異なる値が 0 個または 1 個の場合、対数の項は定数として扱います。

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
| `prev_value(l,r,upper)` / `next_value(l,r,lower)` | `upper` 未満の最大値／`lower` 以上の最小値。返り値は `std::optional<T>`、該当なしは `std::nullopt` | $$O(\log m\log\sigma)$$ |
| `sum_smallest(l,r,k)` / `sum_largest(l,r,k)` | 下位／上位 `k` 個の和 | $$O(\log m\log\sigma)$$ |
| `min_count_smallest(l,r,target)` / `min_count_largest(l,r,target)` | 各順序で和が `target` 以上になる最小個数。届かなければ -1 | $$O(\log m\log\sigma)$$ |
| `max_count_smallest(l,r,budget)` / `max_count_largest(l,r,budget)` | 各順序で和が `budget` 以下の最大個数。空の選択も不可能なら -1 | $$O(\log m\log\sigma)$$ |

`T` はコピーでき、`<` で矛盾なく大小を判定できる型です。互いに `<` で小さくない値は同じキーとしてまとめます。`==` や既定コンストラクタは必要ありません。浮動小数点数の NaN は使わないでください。`bool` は使えません。`Sum` は符号付き整数型です。`PointSetWaveletMatrix<T,Sum>` で型を指定でき、既定の `Sum` は `sizeof(T) < 8` なら `std::int64_t`、それ以外は `__int128_t` です。

比較で同じキーになる値を複数登録した場合、`access`・`kth_*` は登録した値からそのキーの代表値を返します。構造体で比較に使わない情報を持つ場合、その情報まで代入した値と一致するとは限りません。

`range_sum`・`sum_smallest`・`sum_largest`・`min_count_*`・`max_count_*` は、`T` が 64 bit 以下の標準整数型の場合だけ利用できます。中間の部分和も `Sum` に収めてください。

和の境界探索 `min_count_*`・`max_count_*` には初期値と登録した候補全てが非負であることが必要です。前者は `target <= 0` なら 0、後者は `budget < 0` なら -1 を返します。その他のクエリは負の値も扱えます。

構築は $$O(m\log m)$$ 時間、空間は $$O(m\log\sigma)$$ です。更新と区間の順位・和の探索は $$O(\log m\log\sigma)$$、値域を指定しない区間和は $$O(\log m)$$、候補の確認は $$O(\log m)$$、サイズと要素取得は $$O(1)$$ です。要素自身が持つ文字列などの格納領域は別途必要です。
