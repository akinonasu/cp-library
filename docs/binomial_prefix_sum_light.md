---
title: 軽量な二項係数の prefix sum
documentation_of: //math/binomial_prefix_sum_light.hpp
library_category: math/combinatorics
redirect_from:
- /math/combinatorics/binomial_prefix_sum_light.hpp.html
- /lightweight/math/combinatorics/binomial_prefix_sum_light.hpp.html
---

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ を素数 mod で計算します。右端を含みません。$$k\leq0$$ なら 0、$$k\geq n+1$$ なら $$2^n$$ です。

全クエリがそろっている場合は `light_combinatorics::binomial_prefix_sum` で一括計算できます。クエリを 1 件ずつ渡す場合は `light_combinatorics::BinomialPrefixSum` を使います。

直前のクエリとの差分で更新する場合は [差分更新版](https://qandori.github.io/cp-library/math/binomial_prefix_sum_incremental_light.hpp.html) を使えます。

`light_combinatorics` 名前空間で使います。階乗・逆階乗は通常のループで生成します。既定の `mint` の長い積和は AVX-512F・AVX2 対応環境で SIMD を利用します。他の modint にも対応し、通常版と同時に include できます。

```cpp
#include "lib/math/binomial_prefix_sum_light.hpp"
#include <cassert>

int main() {
	light_combinatorics::BinomialPrefixSum<> sum(10000);
	auto a = sum.query(5, 3); // 16
	auto b = sum.prod(5, 1, 3); // 15

	assert(a == mint(16) && b == mint(15));
}
```

一括計算では、入力順に対応する答えを返します。

```cpp
#include "lib/math/binomial_prefix_sum_light.hpp"
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

int main() {
	std::vector<std::pair<int, std::int64_t>> queries{ {5, 3}, {5, 2}, {6, 2}, {5, 6}, {7, 4}, {5, 3} };
	auto values = light_combinatorics::binomial_prefix_sum(queries);
	assert((values == std::vector<mint>{16, 6, 7, 32, 64, 16}));
}
```

`light_combinatorics::binomial_prefix_sum<Mint = mint, K = int>(queries, block_size = 0)` は `std::vector<Mint>` を返します。`queries` は `std::vector<std::pair<int, K>>` で、各要素は `{n,k}` です。`K` は 64 bit 以下の符号付き整数型とし、`n` は非負で `INT_MAX` と法より小さくします。`block_size` は非負の `int` です。添字 `i` の返り値が `queries[i]` の答えになり、重複クエリもそのまま残します。空入力には空の列を返し、入力は変更しません。`Mint` の条件はクラス版と同じです。

一括計算は端や中央の直接答えられるクエリを先に処理し、残ったクエリに必要な範囲だけ階乗と格子を作ります。残ったクエリの `n` がすべて等しい場合は、その行の必要な prefix を一度だけ計算し、格子を作りません。

格子を使う場合、残ったクエリの最大 `n` を $$M$$、個数を $$R$$ とすると、`block_size == 0` では $$\displaystyle B_0=\max(1,\operatorname{round}(M/\sqrt R))$$ を基準にします。クエリの標本から前計算と補正量を見積もり、$$B_0/2\leq B\leq B_0$$ の範囲で幅を選びます。正の `block_size` は指定した幅を使います。直接答えられる場合と同じ行だけの場合は、指定幅にかかわらず格子を省きます。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_combinatorics::BinomialPrefixSum<Mint>(max_n,block_size=0)` | `0 <= n <= max_n` のクエリを前計算。`Mint` の既定は `mint`、`max_n` の既定は 0 | $$O(N+N^2/B)$$ |
| `build(max_n,block_size=0)` | 範囲とブロック幅を変更して前計算を作り直す。返り値は `void` | $$O(N+N^2/B)$$ |
| `query(n,k)` | $$S(n,k)$$ を `Mint` で返す。`k` は `std::int64_t` | $$O(B)$$ |
| `prod(n,l,r)` | $$\displaystyle \sum_{i=l}^{r-1}\binom ni$$ を `Mint` で返す。`l`・`r` は `std::int64_t`、`l <= r` が必要。空区間は 0、範囲外の添字の項も 0 | $$O(B)$$ |
| `max_n()` | 前計算した $$n$$ の上限を `int` で返す | $$O(1)$$ |
| `block_size()` | 使用中のブロック幅を `int` で返す | $$O(1)$$ |
| `table_size()` | 二次元格子に保存した `Mint` の個数を `std::size_t` で返す。階乗などの配列は含まない | $$O(1)$$ |

`BinomialPrefixSum` は $$n$$ の上限を指定して構築します。`query`・`prod` はクエリを 1 個ずつ渡して使えます。呼び出し順序は自由で、`const` なオブジェクトからも取得できます。同じクエリも繰り返し呼び出せます。

`max_n` は非負で `INT_MAX` 未満、法より小さい必要があります。`block_size` は非負で、0 の場合は `max_n + 1` の平方根を切り捨て、1 以上にした幅を使います。大きくすると前計算の保存量が減り、クエリ時に直接足す項が増えます。`query`・`prod` の `n` は前計算の範囲内で指定します。

`Mint` は素数 mod の modint です。`mod()`・`inv()`・整数からの構築・加減乗算に対応し、既定構築で 0 になる型を指定できます。使用中に法を変更しないでください。等値比較は不要です。既存の `init_comb` や `COMB_MAX` は使いません。

`max_n` を $$N$$、ブロック幅を $$B$$、クエリ数を $$Q$$ とします。構築・`build` は $$O(N+N^2/B)$$ 時間、$$O(N+N^2/B^2)$$ 空間、`query`・`prod` は $$O(B)$$、その他のクエリは $$O(1)$$ です。既定の幅では構築が $$\displaystyle O(N\sqrt N)$$ 時間、空間が $$O(N)$$、クエリが $$\displaystyle O(\sqrt N)$$ です。クエリ $$Q$$ 個を処理する合計時間は $$O(N+N^2/B+QB)$$ です。法上の演算を定数時間とし、前計算には定数回の逆元計算を含みます。

クエリ数 $$Q>0$$ が事前に分かる場合、ブロック幅の目安は $$\displaystyle B=\max(1,\lfloor N/\sqrt Q\rfloor)$$ です。クエリが少ないときは幅を大きくすることで前計算を軽くできます。次の例は独立したプログラムです。

```cpp
#include "lib/math/binomial_prefix_sum_light.hpp"
#include <algorithm>
#include <cmath>

int main() {
	const int max_n = 200000, queries = 64;
	int block = std::max(1, int(max_n / std::sqrt(double(queries))));
	light_combinatorics::BinomialPrefixSum<> sum(max_n, block);
	auto value = sum.query(12345, 100);
	(void)value;
}
```

格子と正規化は [kemuniku の実装](https://judge.yosupo.jp/submission/413314)、一括処理は [to-omer/competitive-library](https://github.com/to-omer/competitive-library) を参考にしています。

一括計算では、全クエリの最大 `n` を $$N$$、全クエリ数を $$Q$$ として、時間は $$O(N+Q+M^2/B+RB)$$、空間は $$O(N+Q+M^2/B^2)$$ です。幅の自動選択では $$O(N+Q+M\sqrt R)$$ 時間、$$O(N+Q)$$ 空間になります。格子を省く場合は $$O(N+Q)$$ 時間・空間、空入力は $$O(1)$$ 時間・空間です。
