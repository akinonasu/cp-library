---
title: "二項係数の prefix sum の内部演算"
documentation_of: //components/binomial_prefix_core.hpp
---

[二項係数の prefix sum](https://qandori.github.io/cp-library/math/binomial_prefix_sum.hpp.html) の共通部品です。`binomial_prefix_detail::Table<Mint>` を使います。

| 操作・メンバ | 内容・返り値 |
| --- | --- |
| `Table(n=0)` | $$0\leq i\leq n$$ の階乗・逆階乗・2 の冪を前計算 |
| `build(n)` | 前計算を作り直す |
| `binom(n,k)` | $$\binom nk$$。`k < 0` または `k > n` なら 0 |
| `center(n)` | $$\sum_{i=0}^{\lfloor(n+1)/2\rfloor-1}\binom ni$$ |
| `range(n,l,r)` | $$\sum_{i=l}^{r-1}\binom ni$$。`0 <= l <= r <= n+1` が必要 |
| `factorial` | 階乗の `vector<Mint>` |
| `inverse_factorial` | 逆階乗の `vector<Mint>` |
| `power2` | 2 の冪の `vector<Mint>` |
| `half` | 奇素数 mod では 2 の逆元 |

法は素数、前計算の上限は非負で `INT_MAX` 未満かつ法未満にします。各操作の `n` は前計算の範囲内で指定します。`Mint` の条件は公開ライブラリと同じです。

`Mint = fps_mint` では FPS の階乗・逆階乗生成を使い、他の型ではスカラー処理を使います。

前計算は $$O(n)$$ 時間・空間と逆元計算 1 回です。`binom`・`center` は $$O(1)$$、`range` は $$O(r-l)$$ 時間・$$O(1)$$ 追加空間です。
