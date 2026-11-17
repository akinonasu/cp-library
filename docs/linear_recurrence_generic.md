---
title: 線形漸化式（係数型・畳み込み指定）
documentation_of: //fps/linear_recurrence_generic.hpp
library_category: fps
---

初期値と係数に加え、畳み込みを計算する関数を渡して線形漸化式の第 `k` 項を求めます。法 998244353 以外の modint、合成数を法とする modint、整数、有理数などに使えます。係数を別の型や法に変換することはなく、逆元・除算も使いません。

法 998244353 だけを使う場合は、畳み込みの指定が不要な[通常版](linear_recurrence.md)・[軽量版](fps_light_algorithms.md)も選べます。このヘッダーは単独で使い、必要な畳み込みのヘッダーを別に include します。

```cpp
#include "lib/fps/linear_recurrence_generic.hpp"
#include "lib/convolution/convolution_mod.hpp"
#include "lib/math/modint.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
	using M = StaticModint<1000000007>;
	std::vector<M> initial{0, 1}, coefficients{1, 1};
	auto convolve = [](const std::vector<M> &a, const std::vector<M> &b) {
		std::vector<std::uint32_t> x, y;
		for (auto v : a) x.push_back(v.val());
		for (auto v : b) y.push_back(v.val());
		auto product = convolution_mod(x, y, M::mod());
		std::vector<M> result;
		for (auto v : product) result.push_back(M::raw(v));
		return result;
	};
	assert(linear_recurrence(initial, coefficients, 10, convolve) == M(55));
}
```

初期値・係数の要素型を `T`、個数を $$d$$、求める項番号を $$k$$ とします。式の $$n$$ は $$d$$ 以降の項番号です。長さがそれぞれ $$d+1$$ 以下の 2 列の畳み込みについて、渡す関数の時間の上界を $$C(d)$$、入出力を除く追加空間の上界を $$S(d)$$ とします。係数のコピー・加減算は定数時間として示します。

| 操作 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `linear_recurrence(initial,c,k,convolve)` | $$\displaystyle a_n=\sum_{j=0}^{d-1}c_j a_{n-j-1}$$ の $$a_k$$ を入力と同じ型 `T` で返す | $$O(C(d)\log k)$$ | $$O(d+S(d))$$ |

`initial` と `c` は、同じ要素型 `T` の `std::vector<T,Allocator>` で渡します。長さは同じで、`INT_MAX` 以下にしてください。`initial[i]` は `a[i]`、`c[0]` は一つ前の項に掛ける係数です。指定した初期値は変更せず、漸化式は項番号 `d` から適用します。末尾に 0 の係数があっても同じ意味です。`k` は `long long` の非負整数で、0 始まりの項番号です。既定引数はありません。入力は変更しません。

`k < d` なら指定済みの初期値を、時間・追加空間とも $$O(1)$$ で返し、畳み込み関数は呼びません。次数 `d = 0` なら全ての項を `T(0)` とし、時間・追加空間とも $$O(1)$$ です。計算量の対数は `k = 1` では定数として扱います。

`convolve` は `convolve(const std::vector<T> &a,const std::vector<T> &b)` の形で呼べるものを渡します。2 列を変更せず、通常の畳み込みを `std::vector<T>` で返してください。要素 `i` は、添字の和が `i` となる積 `a[j]*b[i-j]` の総和です。呼び出す 2 列は常に非空で、返り値の長さは `a.size()+b.size()-1` です。最大で長さ `d+1` 同士の畳み込みを使い、返り値の長さは `2*d+1` になります。畳み込み関数が定める入力長・値域・法などの条件も満たしてください。通常版・軽量版のどちらの畳み込み関数も渡せます。

`T` はコピー・代入でき、`T(0)`・`T(1)` で 0 と 1 を構築できる型にします。加減算・乗算は通常の数と同じ等式を満たす必要があります。具体的には、加算・乗算は括弧の付け方を変えても同じ結果になり、`a+b == b+a`・`a*b == b*a`・`a*(b+c) == a*b+a*c` が成り立つものにします。0 の加算と 1 の乗算は値を変えず、`a-a` と 0 を掛けた結果は 0 です。型の比較演算子や、0 でない全ての値で割れることは必要ありません。

符号付き整数型や固定幅の有理数型では、畳み込みを含む途中の演算も型の範囲に収めてください。符号なし整数型は、その型のビット幅を $$w$$ とすると、通常の加減乗算によって法 $$2^w$$ の計算にも使えます。浮動小数点型を使った場合の丸め誤差は取り除きません。

例えば、以下は整数係数に単純な畳み込みを指定する例です。小さい次数や、独自の係数型を使うときにも同じ形で指定できます。

```cpp
#include "lib/fps/linear_recurrence_generic.hpp"
#include <cassert>
#include <vector>

int main() {
	std::vector<long long> initial{0, 1}, coefficients{1, 1};
	auto convolve = [](const std::vector<long long> &a, const std::vector<long long> &b) {
		std::vector<long long> result(a.size()+b.size()-1, 0);
		for (std::size_t i = 0; i < a.size(); ++i)
			for (std::size_t j = 0; j < b.size(); ++j) result[i+j] += a[i]*b[j];
		return result;
	};
	assert(linear_recurrence(initial, coefficients, 10, convolve) == 55);
}
```

畳み込みが $$O(d\log d)$$ 時間・$$O(d)$$ 追加空間なら、全体は $$O(d\log d\log k)$$ 時間・$$O(d)$$ 追加空間です。単純な畳み込みなら $$O(d^2\log k)$$ 時間・$$O(d)$$ 追加空間です。次数 1 の畳み込みは定数時間です。呼び出し後に前計算や共有表を保持しません。渡す畳み込み関数が表などを保持する場合は、その関数の説明に従います。

参考：[ei1333：Kth Term of Linearly Recurrent Sequence](https://github.com/ei1333/library/blob/master/math/fps/kth-term-of-linearly-recurrent-sequence.hpp)、[suisen：Bostan–Mori](https://github.com/suisen-cp/cp-library-cpp/blob/main/library/polynomial/bostan_mori.hpp)
