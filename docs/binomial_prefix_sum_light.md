---
title: 軽量な二項係数の prefix sum
documentation_of: //math/binomial_prefix_sum_light.hpp
library_category: math/combinatorics
redirect_from:
- /math/combinatorics/binomial_prefix_sum_light.hpp.html
- /lightweight/math/combinatorics/binomial_prefix_sum_light.hpp.html
---

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ を素数 mod で計算します。右端を含みません。$$k\leq0$$ なら 0、$$k\geq n+1$$ なら $$2^n$$ です。

直前のクエリとの差分で更新する場合は [差分更新版](https://qandori.github.io/cp-library/math/binomial_prefix_sum_incremental_light.hpp.html) を使えます。

`light_combinatorics` 名前空間で使います。一括版の多項式演算は `light_fps` に基づき、通常版 FPS は読み込みません。階乗・逆階乗は通常のループで生成します。他の modint にも対応します。通常版と同時に include できます。

```cpp
#include "lib/math/binomial_prefix_sum_light.hpp"
#include <cassert>

int main() {
	light_combinatorics::BinomialPrefixSum<> sum(10000);
	auto a = sum.query(5, 3); // 16
	auto b = sum.prod(5, 1, 3); // 15

	std::vector<std::pair<int, int>> queries;
	queries.emplace_back(5, 3);
	queries.emplace_back(4, 1);
	auto answers = light_combinatorics::binomial_prefix_sum(queries);
	assert(a == mint(16) && b == mint(15));
	assert(answers.size() == 2 && answers[0] == mint(16) && answers[1] == mint(1));
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `light_combinatorics::BinomialPrefixSum<Mint>(max_n,block_size=0)` | `0 <= n <= max_n` のクエリを前計算。`Mint` の既定は `mint`、`max_n` の既定は 0 | $$O(N+N^2/B)$$ |
| `build(max_n,block_size=0)` | 範囲とブロック幅を変更して前計算を作り直す。返り値は `void` | $$O(N+N^2/B)$$ |
| `query(n,k)` | $$S(n,k)$$ を `Mint` で返す。`k` は `std::int64_t` | $$O(B)$$ |
| `prod(n,l,r)` | $$\displaystyle \sum_{i=l}^{r-1}\binom ni$$ を `Mint` で返す。`l`・`r` は `std::int64_t`、`l <= r` が必要。空区間は 0、範囲外の添字の項も 0 | $$O(B)$$ |
| `max_n()` | 前計算した $$n$$ の上限を `int` で返す | $$O(1)$$ |
| `block_size()` | 使用中のブロック幅を `int` で返す | $$O(1)$$ |
| `table_size()` | 二次元格子に保存した `Mint` の個数を `std::size_t` で返す。階乗などの配列は含まない | $$O(1)$$ |
| `light_combinatorics::binomial_prefix_sum<Mint>(queries)` | `(n,k)` の列を一括処理し、入力順に $$S(n,k)$$ の `vector<Mint>` を返す。`Mint` は省略可能 | $$O(N+Q\log Q+D\log^2 D)$$（既定の `mint`） |

`BinomialPrefixSum` は $$n$$ の上限だけを指定して構築します。`query`・`prod` はオンラインで使え、クエリの先読みは不要です。呼び出し順序は自由で、`const` なオブジェクトからも取得できます。一括版の `binomial_prefix_sum(queries)` は全クエリを先読みして渡します。オンライン版は $$n$$ と $$k$$ の二次元格子に正規化した prefix を保存し、近い格子点・端・中央からの差分を計算します。一括版は降階乗基底の多項式を分割統治で評価します。一括版のアルゴリズムは入力分布によって切り替えません。どちらも対称性と中央の prefix を使います。

`max_n` は非負で `INT_MAX` 未満、法より小さい必要があります。`block_size` は非負で、0 の場合は `max_n + 1` の平方根を切り捨て、1 以上にした幅を使います。大きくすると前計算の保存量が減り、クエリ時に直接足す項が増えます。`query`・`prod` の `n` は前計算の範囲内で指定します。

一括版の `queries` は `vector<pair<int,K>>` で、`K` は 64 bit 以下の符号付き整数型です。クエリは重複していてもよく、答えは入力の各位置に対応します。空の入力には空の `vector<Mint>` を返します。各 `n` は非負で、最大値は `INT_MAX` 未満かつ法未満、クエリ数は `INT_MAX` 以下にします。

`Mint` は素数 mod の modint です。`mod()`・`inv()`・整数からの構築・加減乗算に対応し、既定構築で 0 になる型を指定できます。使用中に法を変更しないでください。既存の `init_comb` や `COMB_MAX` は使いません。

`max_n` を $$N$$、ブロック幅を $$B$$、クエリ数を $$Q$$ とします。オンライン版の構築・`build` は $$O(N+N^2/B)$$ 時間、$$O(N+N^2/B^2)$$ 空間、`query`・`prod` は $$O(B)$$、その他のクエリは $$O(1)$$ です。既定の幅では構築が $$\displaystyle O(N\sqrt N)$$ 時間、空間が $$O(N)$$、クエリが $$\displaystyle O(\sqrt N)$$ です。一括版では、空・全体・端・中央を直接処理し、残ったクエリを $$m=\min(k,n+1-k)$$ に折り返します。同じ $$(m,n)$$ をまとめ、その個数を $$U$$、$$m$$ の最大値を $$M$$、$$D=M+U$$ とします。残りがなければ $$D=0$$ です。既定の `mint` で NTT を使える大きさでは $$O(N+Q\log Q+D\log^2 D)$$ 時間、$$O(N+Q+D\log D)$$ 空間です。他の `Mint` と NTT の上限を超える演算には Karatsuba 法を使い、時間は $$O(N+Q\log Q+D^{\log_2 3}\log D)$$ です。いずれも法上の演算を定数時間とした評価で、階乗の前計算に逆元計算 1 回を含みます。

オンライン版の格子と正規化は [kemuniku の実装](https://judge.yosupo.jp/submission/413314) を参考にしています。
