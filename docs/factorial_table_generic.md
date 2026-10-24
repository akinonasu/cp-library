---
title: 汎用 modint の階乗・逆階乗生成
documentation_of: //components/factorial_table_generic.hpp
---

通常版・軽量版の[二項係数](comb.md)が共有する表の生成処理です。利用者向けの階乗・逆階乗は二項係数ライブラリから取得できます。

`factorial_table_detail::build<Mint, Canonical = true>(factorial,inverse_factorial,n)` は、各 `Mint*` の先頭 `n+1` 要素に階乗・逆階乗を書き込みます。領域は重ならない有効な配列とし、素数法 `p = Mint::mod()` に対して `0 <= n < p` を満たしてください。

`Mint` は整数からの構築・乗算・`inv()` に対応させます。`Canonical = true` かつ整数の `val()` を持つ型では、その剰余代表を使えるものとして扱います。`false` では型自身の乗算を使います。表を生成している間は法を変更しません。

逆元計算を $$O(\log p)$$ として、時間は $$O(n+\log p)$$、出力配列を除く追加空間は $$O(1)$$ です。
