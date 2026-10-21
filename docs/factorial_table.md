---
title: 型を指定する階乗・逆階乗テーブル
documentation_of: //components/factorial_table.hpp
---

modint 型ごとに階乗・逆階乗を構築する内部部品です。通常は[二項係数ライブラリ](comb.md)を利用します。

`factorial_detail::Table<Mint>(max_n)` は、素数 `Mint::mod()` に対し `0` から `min(max_n,Mint::mod()-1)` まで前計算します。`factorial[i]` が階乗、`inverse_factorial[i]` が逆階乗、`modulus` が構築時の法です。構築後の modint の法は変更しないでください。変更した場合は新しいテーブルを構築します。

`Mint` は整数からの構築、乗算、`mod()` と `inv()` に対応するものを使います。`max_n` は `0` 以上 `INT_MAX` 未満とします。

自作の `fps_mint` では既存の SIMD 前計算を再利用します。その他の型では階乗を前から計算し、末尾の逆元を 1 回だけ計算して逆階乗を後ろから求めます。

実際の前計算上限を $$N$$、法を $$p$$ とすると、構築は $$O(N+\log p)$$ 時間、$$O(N)$$ 空間です。modint の乗算を $$O(1)$$、逆元計算を $$O(\log p)$$ とします。
