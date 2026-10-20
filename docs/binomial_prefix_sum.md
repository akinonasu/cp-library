---
title: 二項係数の prefix sum
documentation_of: //math/combinatorics/binomial_prefix_sum.hpp
redirect_from:
- /math/binomial_prefix_sum.hpp.html
---

$$\displaystyle S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ を素数 mod で計算します。右端を含みません。$$k\leq0$$ なら 0、$$k\geq n+1$$ なら $$2^n$$ です。

直前の問い合わせとの差分で更新する場合は [差分更新版](binomial_prefix_sum_incremental.hpp.html) を使えます。

既定の `mint` では FPS と階乗・逆階乗の生成を共有し、AVX2 に対応していれば SIMD を使います。他の modint も従来どおり利用できます。

```cpp
#include "lib/math/combinatorics/binomial_prefix_sum.hpp"

BinomialPrefixSum<> sum(10000);
auto a = sum.query(5, 3); // 16
auto b = sum.prod(5, 1, 3); // 15

std::vector<std::pair<int, int>> queries;
queries.emplace_back(5, 3);
queries.emplace_back(4, 1);
auto answers = binomial_prefix_sum(queries);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `BinomialPrefixSum<Mint>(max_n,block_size=0)` | $$0\leq n\leq\text{max\_n}$$ の問い合わせを前計算。`Mint` の既定は `mint`、`max_n` の既定は 0 | $$O(N+N^2/B)$$ |
| `build(max_n,block_size=0)` | 範囲とブロック幅を変更して前計算を作り直す | $$O(N+N^2/B)$$ |
| `query(n,k)` | $$S(n,k)$$ を `Mint` で返す。`k` は `std::int64_t` | $$O(B)$$ |
| `prod(n,l,r)` | $$\displaystyle \sum_{i=l}^{r-1}\binom ni$$ を返す。`l <= r` が必要。範囲外の添字の項は 0 | $$O(B)$$ |
| `max_n()` | 前計算した $$n$$ の上限 | $$O(1)$$ |
| `block_size()` | 使用中のブロック幅 | $$O(1)$$ |
| `table_size()` | ブロック境界の前計算に保存した `Mint` の個数。階乗などの配列は含まない | $$O(1)$$ |
| `binomial_prefix_sum<Mint>(queries)` | `(n,k)` の列を一括処理し、入力順に $$S(n,k)$$ の `vector<Mint>` を返す。`Mint` は省略可能 | $$O(N+Q\log Q+D\log^2(D+1))$$（既定の `mint`） |

`BinomialPrefixSum` の `query`・`prod` はオンラインで使え、クエリの先読みは不要です。一括版の `binomial_prefix_sum(queries)` は全クエリを先読みして渡します。オンライン版は平方分割、一括版は降階乗基底の多項式を分割統治で評価します。一括版のアルゴリズムは入力分布によって切り替えません。どちらも対称性と中央の prefix を使います。

`max_n` は非負で `INT_MAX` 未満、法より小さい必要があります。`block_size` は非負で、0 の場合は $$\displaystyle \max(1,\lfloor\sqrt{\text{max\_n}+1}\rfloor)$$ を使います。大きくすると前計算の保存量が減り、問い合わせ時に直接足す項が増えます。`query`・`prod` の `n` は前計算の範囲内で指定します。

一括版の `queries` は `vector<pair<int,K>>` で、`K` は 64 bit 以下の符号付き整数型です。空の入力にも対応します。各 `n` は非負で、最大値は `INT_MAX` 未満かつ法未満、問い合わせ数は `INT_MAX` 以下にします。

`Mint` は素数 mod の modint です。`mod()`・`inv()`・整数からの構築・加減乗算に対応し、既定構築で 0 になる型を指定できます。使用中に法を変更しないでください。既存の `init_comb` や `COMB_MAX` は使いません。既定の `mint` では、残りの項の積を 128 bit 整数にまとめて加算します。

$$N=\text{max\_n}$$、ブロック幅を $$B$$、問い合わせ数を $$Q$$ とします。オンライン版の構築・`build` は時間・空間とも $$O(N+N^2/B)$$、`query`・`prod` は $$O(B)$$、その他の問い合わせは $$O(1)$$ です。既定の幅では構築・空間が $$\displaystyle O(N\sqrt N)$$、問い合わせが $$\displaystyle O(\sqrt N)$$ です。一括版では、空・全体・端・中央を直接処理し、残った問い合わせを $$m=\min(k,n+1-k)$$ に折り返します。同じ $$(m,n)$$ をまとめ、その個数を $$U$$、$$m$$ の最大値を $$M$$、$$D=M+U$$ とします。残りがなければ $$D=0$$ です。既定の `mint` で NTT を使える大きさでは $$O(N+Q\log Q+D\log^2(D+1))$$ 時間、$$O(N+Q+D\log(D+1))$$ 空間です。他の `Mint` と NTT の上限を超える演算には Karatsuba 法を使い、時間は $$O(N+Q\log Q+D^{\log_2 3}\log(D+1))$$ です。いずれも法上の演算を定数時間とした評価で、階乗の前計算に逆元計算 1 回を含みます。
