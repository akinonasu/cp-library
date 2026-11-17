---
title: 個数指定の部分和集合
documentation_of: //math/fixed_size_subset_sum.hpp
library_category: math/combinatorics
---

整数の集合から異なる要素をちょうど `k` 個選び、作れる和をすべて求めます。選び方の個数ではなく、和の集合を返します。

大きな入力には NTT を使い、入力によっては単純な列挙で答えを求めます。

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

入力の長さを $$M$$、異なる値の個数を $$m$$、出力の長さを $$R$$ とします。$$0\le k\le m$$ の場合、$$K=\min(k,m-k)$$ とします。NTT 経路では、各値から最小値を引き、全ての差の最大公約数で割った値を使います。その値の大きい方から $$K$$ 個の和を $$H$$、$$H+1$$ 以上の最小の 2 の累乗を $$L$$、使用する素数の個数を $$P\le10$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fixed_size_subset_sums(values,k)` | 整数列と非負の `int` をこの順に渡す。作れる和を昇順・重複なしの `std::vector<long long>` で返す。既定引数なし | NTT 経路は $$O(M\log M+PL(\log L+K^2)+R)$$ |

`values` は `std::vector<T,Allocator>` または `std::span<T,Extent>` / `std::span<const T,Extent>` で渡します。`span` は固定長・可変長の両方に対応します。`T` は `bool` を除く 64 bit 以下の標準整数型とし、負数や型の最小値・最大値も使えます。返り値の要素型は入力型によらず `long long` です。作れる和のいずれかが `long long` の範囲を超える場合は `std::overflow_error` を送出します。入力の順序は自由で、入力元の配列は変更しません。重複した値は一つにまとめます。同じ値を複数回使いたい場合の多重集合には対応しません。入力の長さは `INT_MAX` 以下にします。

`k = 0` なら空集合を選んだ和として `{0}` を返します。`k` が異なる値の個数を超える場合は空配列です。空入力でもこの規則を使います。値 `0` も一つの要素として選べます。和が 32 bit を超える場合も `long long` で返します。

```cpp
#include "lib/math/fixed_size_subset_sum.hpp"
#include <cassert>
#include <vector>

int main() {
	std::vector<long long> values{-1000000000000LL, 2, 7};
	auto sums = fixed_size_subset_sums(values, 2);
	assert((sums == std::vector<long long>{-999999999998LL, -999999999993LL, 9}));
}
```

選ぶ個数が多い場合は、選ばない $$K$$ 個の和を全体の和から引きます。$$K=0,1$$、組合せ数が 32768 以下、または値が等差数列をなす場合は、NTT を使わずに求めます。

それ以外は、上で定義した圧縮後の値を使います。$$H<2^{23}$$ が必要です。また、係数の上限 $$\displaystyle B=\left\lfloor\frac{\binom m{K-1}}K\right\rfloor$$ が、[内部部品](fixed_size_subset_sum_core.md)で使う 10 個の NTT 用の素数の積未満である必要があります。NTT が必要な入力でどちらかを満たさない場合は `std::length_error` を送出します。例えば値が `0` から `200000` の範囲で `k <= 16` なら、両条件を満たします。

複数の素数を法として、それぞれの和を作る選び方の数が 0 かどうかを計算します。一つでも 0 でない結果を得た和は必ず作れます。結果がすべて 0 の和は、選び方の数の上限と使用した素数の積を比較して判定します。必要に応じて素数を追加するため、乱数による誤判定や、選び方の数が法の倍数になることによる取りこぼしはありません。

NTT 経路は $$O(M\log M+PL(\log L+K^2)+R)$$ 時間、$$O(M+L+K+R)$$ 空間です。各素数につき順変換と逆変換を一度ずつ行います。組合せ数 $$E=\binom mK$$ が 32768 以下なら $$O(M\log M+KE+E\log E)$$ 時間、$$O(M+K+E)$$ 空間です。等差数列なら $$O(M\log M+R)$$ 時間、$$O(M+R)$$ 空間です。`k = 0` は入力のコピーを除き $$O(1)$$ 時間・空間です。`vector` の変数をそのまま渡す場合は入力をコピーして受け取り、その分の $$O(M)$$ 時間・空間を含みます。
