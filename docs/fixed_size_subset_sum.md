---
title: 個数指定の部分和集合
documentation_of: //math/fixed_size_subset_sum.hpp
library_category: math/combinatorics
---

非負整数の集合から異なる要素をちょうど `k` 個選び、作れる和をすべて求めます。選び方の個数ではなく、和の集合を返します。

```cpp
#include "lib/math/fixed_size_subset_sum.hpp"
#include <iostream>

int main() {
	std::vector<int> values{0, 2, 5, 8};
	auto sums = fixed_size_subset_sums(values, 2);
	for (std::size_t i = 0; i < sums.size(); ++i)
		std::cout << (i ? " " : "") << sums[i];
	std::cout << '\n';
}
```

出力は `2 5 7 8 10 13` です。返り値は昇順なので、`std::binary_search(sums.begin(), sums.end(), x)` で和 `x` を作れるか調べられます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fixed_size_subset_sums(values,k)` | `std::vector<int>` と非負の `int` をこの順に渡す。作れる和を昇順・重複なしの `std::vector<long long>` で返す。既定引数なし | NTT 経路は $$O(M\log M+PL(\log L+K^2)+R)$$ |

`values` の要素は `0` 以上 `INT_MAX` 以下です。入力の順序は自由で、入力元の配列は変更しません。重複した値は一つにまとめます。同じ値を複数回使いたい場合の多重集合には対応しません。入力の長さは `INT_MAX` 以下にします。

`k = 0` なら空集合を選んだ和として `{0}` を返します。`k` が異なる値の個数を超える場合は空配列です。空入力でもこの規則を使います。値 `0` も一つの要素として選べます。和が 32 bit を超える場合も `long long` で返します。

異なる値の個数を $$m$$、$$K=\min(k,m-k)$$ とします。選ぶ個数が多い場合は、選ばない $$K$$ 個の和を全体の和から引きます。$$K=0,1$$、組合せ数が 32768 以下、または値が等差数列をなす場合は、NTT を使わずに求めます。

それ以外は、最小値を引き、全要素の差の最大公約数で割って座標を圧縮します。この座標で上位 $$K$$ 個の和を $$H$$、$$L$$ を $$H+1$$ 以上の最小の 2 冪とします。$$H<2^{23}$$ が必要です。また、係数の上限 $$\displaystyle B=\left\lfloor\frac{\binom m{K-1}}K\right\rfloor$$ が、[内部部品](fixed_size_subset_sum_core.md)で使う 10 個の NTT 素数の積未満である必要があります。NTT が必要な入力でどちらかを満たさない場合は `std::length_error` を送出します。例えば値が `0` から `200000` の範囲で `k <= 16` なら、両条件を満たします。

有限体での数え上げには Newton の恒等式を使います。一つでも非ゼロの余りを持つ和は必ず作れます。余りがすべて 0 の和は、係数の上限と使用した素数の積を比較して判定します。必要に応じて素数を追加するため、乱数による誤判定や、係数が法の倍数になることによる取りこぼしはありません。

入力の長さを $$M$$、出力の長さを $$R$$、使用した素数の個数を $$P\leq10$$ とすると、NTT 経路は $$O(M\log M+PL(\log L+K^2)+R)$$ 時間、$$O(M+L+K+R)$$ 空間です。各素数につき順変換と逆変換を一度ずつ行います。組合せ数 $$E=\binom mK$$ が 32768 以下なら $$O(M\log M+KE+E\log E)$$ 時間、$$O(M+K+E)$$ 空間です。等差数列なら $$O(M\log M+R)$$ 時間、$$O(M+R)$$ 空間です。`k = 0` は入力の検査のみで $$O(M)$$ 時間です。
