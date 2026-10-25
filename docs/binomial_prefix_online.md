---
title: 二項係数 prefix sum のオンライン処理部品
documentation_of: //components/binomial_prefix_online.hpp
---

[二項係数の prefix sum](https://qandori.github.io/cp-library/math/binomial_prefix_sum.hpp.html) の通常版・軽量版で共有する内部実装です。利用するときは公開ヘッダーを include してください。

`binomial_prefix_detail::Online<Mint,Table>` は、素数 mod の要素型 `Mint` と階乗・逆階乗・2 の累乗を生成する `Table` を使います。二次元格子に正規化した prefix を保存し、近い格子点から積和で補正します。`max_n` は非負かつ `INT_MAX` と法より小さく、`block_size` は非負にします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `Online(max_n=0,block_size=0)` | 上限とブロック幅を指定して構築。幅 0 は平方根を基準に決定 | $$O(N+N^2/B)$$ |
| `build(max_n,block_size=0)` | 前計算を作り直す。返り値は `void` | $$O(N+N^2/B)$$ |
| `query(n,k)` | 右端を含まない prefix を `Mint` で返す | $$O(B)$$ |
| `prod(n,l,r)` | `l <= r` の区間和を `Mint` で返す | $$O(B)$$ |
| `max_n()` | 上限を `int` で返す | $$O(1)$$ |
| `block_size()` | ブロック幅を `int` で返す | $$O(1)$$ |
| `table_size()` | 格子に保存した `Mint` の個数を `std::size_t` で返す | $$O(1)$$ |

`n` は前計算した範囲内で指定し、`k`・`l`・`r` は `std::int64_t` です。負の添字と `n` より大きい添字の二項係数は 0 として扱います。上限 0 にも対応します。

上限を $$N$$、ブロック幅を $$B$$ とすると、構築は $$O(N+N^2/B)$$ 時間、保存量は $$O(N+N^2/B^2)$$、取得は $$O(B)$$ 時間です。
