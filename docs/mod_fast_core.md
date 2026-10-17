---
title: "定数時間 mod 演算の内部部品"
documentation_of: //components/mod_fast_core.hpp
---

[ModInv](https://qandori.github.io/cp-library/math/mod_inv.hpp.html) と [ModPow](https://qandori.github.io/cp-library/math/mod_pow.hpp.html) が使う内部部品です。通常は各ライブラリのヘッダーを include します。名前空間は `mod_fast_detail` です。

| 操作 | 内容・返り値 |
| --- | --- |
| `Reducer(m)` | $$1\le m<2^{31}$$ の法を指定し、逆数を前計算 |
| `Reducer::reduce(x)` | 任意の `uint64_t` の剰余を `uint32_t` で返す |
| `Reducer::mul(a,b)` | `uint32_t` の積の剰余 |
| `Reducer::pow(a,e)` | 非負の `uint64_t` 指数による繰り返し二乗法 |
| `Reducer::mod` / `Reducer::reciprocal` | 法／縮約用の逆数。参照用 |
| `RationalTable(p)` | $$2\le p<2^{31}$$ の素数に対する有理数近似の表を構築 |
| `RationalTable::direct()` | 小さい法に対して全要素の表を使うか |
| `RationalTable::limit()` | 分子の絶対値の上限 |
| `RationalTable::fraction(x)` | $$x\equiv a/b\pmod p$$ を満たす `{a,b}`。$$1\le x<p$$、$$a\ne0$$、$$b>0$$ |

`RationalTable` は Farey 数列を走査し、各バケットの中央で分子の絶対値が小さい分数を保存します。分母上限とバケット幅を法に応じて調整し、問い合わせには右シフト・配列参照・積と差だけを使います。

`Reducer` の構築・縮約・積は $$O(1)$$ 時間・空間、累乗は $$O(\log e)$$ 時間です。`RationalTable` の構築は $$O(p^{2/3})$$ 時間・空間、各問い合わせは $$O(1)$$ 時間です。
