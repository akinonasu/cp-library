---
title: 二項係数 prefix sum の一括処理部品
documentation_of: //components/binomial_prefix_batch.hpp
---

[二項係数の prefix sum](https://qandori.github.io/cp-library/math/binomial_prefix_sum.hpp.html) の通常版・軽量版で共有する内部実装です。利用するときは公開ヘッダーの `binomial_prefix_sum` を使ってください。

`binomial_prefix_detail::batch<Mint,Table>(queries,block_size)` は `std::vector<Mint>` を返します。`queries` は `std::vector<std::pair<int,K>>`、`K` は 64 bit 以下の符号付き整数型です。各要素 `{n,k}` に対し、右端を含まない和 $$\displaystyle \sum_{i=0}^{k-1}\binom ni$$ を求め、入力順に返します。`k <= 0` は 0、`k >= n+1` は $$2^n$$ です。重複クエリも保持し、空入力には空の列を返します。入力は変更しません。

`n` は非負かつ `INT_MAX` と法より小さく、`block_size` は非負の `int` とします。`Mint` は素数 mod の要素型、`Table` は同じ要素型の階乗・逆階乗・2 の累乗と区間の二項係数和を扱う型です。通常版の `binomial_prefix_detail::Table` と軽量版の `light_binomial_prefix_detail::Table` を組み合わせられます。

端や中央の直接答えられるクエリを先に処理します。残ったクエリが同じ行にある場合は必要な prefix を一度だけ計算し、異なる行にある場合は `Online<Mint,Table>` の格子を使います。

残ったクエリの最大 `n` を $$M$$、個数を $$R$$ とすると、幅 0 は $$\displaystyle B_0=\max(1,\operatorname{round}(M/\sqrt R))$$ を基準にします。全クエリ数から求めた幅も候補にし、残ったクエリの標本から前計算と補正量を見積もって選びます。選ぶ幅は $$B_0/2\leq B\leq B_0$$ の範囲です。正の幅はそのまま使います。格子を省く場合は幅を使いません。

全クエリの最大 `n` を $$N$$、個数を $$Q$$ とすると、時間は $$O(N+Q+M^2/B+RB)$$、空間は $$O(N+Q+M^2/B^2)$$ です。幅 0 は $$O(N+Q+M\sqrt R)$$ 時間・$$O(N+Q)$$ 空間です。格子を省く場合は $$O(N+Q)$$ 時間・空間、空入力は $$O(1)$$ 時間・空間です。法上の演算を定数時間とし、階乗表には定数回の逆元計算を含みます。
