---
title: 二項係数の prefix sum
documentation_of: //math/binomial_prefix_sum.hpp
redirect_from:
- /math/combinatorics/binomial_prefix_sum.hpp.html
- /lightweight/math/combinatorics/binomial_prefix_sum.hpp.html
library_category: math/combinatorics
---

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ を素数 mod で計算します。右端を含みません。$$k\leq0$$ なら 0、$$k\geq n+1$$ なら $$2^n$$ です。

直前のクエリとの差分で更新する場合は [差分更新版](https://qandori.github.io/cp-library/math/binomial_prefix_sum_incremental.hpp.html) を使えます。

既定の `mint` では共通の階乗・逆階乗生成を使い、AVX2 対応環境では SIMD を利用します。他の素数 modint も指定できます。

```cpp
#include "lib/math/binomial_prefix_sum.hpp"
#include <cassert>

int main() {
	BinomialPrefixSum<> sum(10000);
	auto a = sum.query(5, 3); // 16
	auto b = sum.prod(5, 1, 3); // 15

	assert(a == mint(16) && b == mint(15));
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `BinomialPrefixSum<Mint>(max_n,block_size=0)` | `0 <= n <= max_n` のクエリを前計算。`Mint` の既定は `mint`、`max_n` の既定は 0 | $$O(N+N^2/B)$$ |
| `build(max_n,block_size=0)` | 範囲とブロック幅を変更して前計算を作り直す。返り値は `void` | $$O(N+N^2/B)$$ |
| `query(n,k)` | $$S(n,k)$$ を `Mint` で返す。`k` は `std::int64_t` | $$O(B)$$ |
| `prod(n,l,r)` | $$\displaystyle \sum_{i=l}^{r-1}\binom ni$$ を `Mint` で返す。`l`・`r` は `std::int64_t`、`l <= r` が必要。空区間は 0、範囲外の添字の項も 0 | $$O(B)$$ |
| `max_n()` | 前計算した $$n$$ の上限を `int` で返す | $$O(1)$$ |
| `block_size()` | 使用中のブロック幅を `int` で返す | $$O(1)$$ |
| `table_size()` | 二次元格子に保存した `Mint` の個数を `std::size_t` で返す。階乗などの配列は含まない | $$O(1)$$ |

`BinomialPrefixSum` は $$n$$ の上限を指定して構築します。`query`・`prod` はクエリを 1 個ずつ渡して使えます。呼び出し順序は自由で、`const` なオブジェクトからも取得できます。同じクエリも繰り返し呼び出せます。

`max_n` は非負で `INT_MAX` 未満、法より小さい必要があります。`block_size` は非負で、0 の場合は `max_n + 1` の平方根を切り捨て、1 以上にした幅を使います。大きくすると前計算の保存量が減り、クエリ時に直接足す項が増えます。`query`・`prod` の `n` は前計算の範囲内で指定します。

`Mint` は素数 mod の modint です。`mod()`・`inv()`・整数からの構築・加減乗算に対応し、既定構築で 0 になる型を指定できます。使用中に法を変更しないでください。既存の `init_comb` や `COMB_MAX` は使いません。既定の `mint` では、残りの項の積を 128 bit 整数にまとめて加算します。

`max_n` を $$N$$、ブロック幅を $$B$$、クエリ数を $$Q$$ とします。構築・`build` は $$O(N+N^2/B)$$ 時間、$$O(N+N^2/B^2)$$ 空間、`query`・`prod` は $$O(B)$$、その他のクエリは $$O(1)$$ です。既定の幅では構築が $$\displaystyle O(N\sqrt N)$$ 時間、空間が $$O(N)$$、クエリが $$\displaystyle O(\sqrt N)$$ です。クエリ $$Q$$ 個を処理する合計時間は $$O(N+N^2/B+QB)$$ です。法上の演算を定数時間とし、前計算に逆元計算 1 回を含みます。

クエリ数 $$Q>0$$ が事前に分かる場合、ブロック幅の目安は $$\displaystyle B=\max(1,\lfloor N/\sqrt Q\rfloor)$$ です。クエリが少ないときは幅を大きくすることで前計算を軽くできます。次の例は独立したプログラムです。

```cpp
#include "lib/math/binomial_prefix_sum.hpp"
#include <algorithm>
#include <cmath>

int main() {
	const int max_n = 200000, queries = 64;
	int block = std::max(1, int(max_n / std::sqrt(double(queries))));
	BinomialPrefixSum<> sum(max_n, block);
	auto value = sum.query(12345, 100);
	(void)value;
}
```

格子と正規化は [kemuniku の実装](https://judge.yosupo.jp/submission/413314) を参考にしています。
