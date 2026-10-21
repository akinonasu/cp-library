---
title: "二項係数の prefix sum の多項式評価"
documentation_of: //components/binomial_prefix_polynomial.hpp
---

[二項係数の prefix sum](https://qandori.github.io/cp-library/math/combinatorics/binomial_prefix_sum.hpp.html) の offline 版の内部部品です。通常は公開関数 `binomial_prefix_sum(queries)` を使います。

`binomial_prefix_detail::Polynomial<Mint>(table,points,ids,result)` は、共通の `Table<Mint>` と折り返し済みの `(m,n)` を受け取り、$$S(n,m)$$ を `result` に書き込みます。`points` は空でないこと、`0 <= m <= n+1`、`n` が前計算の範囲内であることが必要です。`ids` の長さは `points` と同じにし、上位 31 bit は書き込み先、最下位 bit は $$2^n-S(n,m)$$ への反転指定にします。

$$\displaystyle P_{a,b}(x)=\prod_{j=a}^{b-1}(x-j),\qquad H_{a,b}(x)=\sum_{i=a}^{b-1}\frac{1}{i!}\prod_{j=a}^{i-1}(x-j)$$

を分割統治で構築します。境界 $$c$$ では $$P_{a,b}=P_{a,c}P_{c,b}$$、$$H_{a,b}=H_{a,c}+P_{a,c}H_{c,b}$$ です。クエリを $$m$$ 順に並べ、子に渡す多項式を、その子が持つクエリの $$\displaystyle \prod(x-n)$$ で剰余化します。根と右端の経路では使わない基底を作らず、葉の最大 16 点を直接評価します。同じ $$(m,n)$$ の結果は共有します。

既定の `mint` には FPS の乗算・多項式剰余を使います。他の `Mint` は加減乗算・逆元だけを使う Karatsuba 乗算と Newton 法で処理し、`val()`・整数への変換・単項マイナス・等値比較を要求しません。既定の `mint` でも NTT の上限を超える演算にはこの演算を使います。Mo 法との切り替えはありません。

前計算済みの $$n$$ の上限を $$N$$、入力点数を $$Q$$、重複を除いた点数を $$U$$、$$m$$ の最大値を $$M$$、$$D=M+U$$ とします。NTT を使える場合は $$O(Q\log Q+D\log^2(D+1))$$ 時間、その他は $$O(Q\log Q+D^{\log_2 3}\log(D+1))$$ 時間です。追加空間は $$O(Q+D\log(D+1))$$、`Table` は別途 $$O(N)$$ 空間です。
