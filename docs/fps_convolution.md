---
title: FPS の融合畳み込み
documentation_of: //components/fps_convolution.hpp
---

Linux x86_64 / GCC の `fps::multiply` と `convolution(vector<mint>,vector<mint>)` が使う内部部品です。公開 API から利用し、呼び出し側が AVX2 対応を実行時に確認します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::multiply_detail::multiply(a,b)` | 内部 Montgomery 係数の二つの非空配列の積を返す | $$O(n\log n)$$ |

通常は [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f*g` を使います。ここで `n` は入出力の最大係数数です。

`fps_detail::multiply_detail::multiply(a,b)` は、非空の Montgomery 係数列から積を返します。出力長は $$2^{23}$$ 以下とし、入力を変更しません。同一配列の二乗にも対応します。入力に alignment や padding は要求しません。

両入力が 4096 項以上で、それぞれの列の全係数が一定であることを正規化した値で厳密に確認できる場合は、[定数係数畳み込み](https://qandori.github.io/cp-library/components/fps_convolution_constant.hpp.html) を使います。重なりの項数から積を直接作り、時間は $$O(n+m)$$、返す配列を除く追加作業領域は $$O(1)$$ です。入力の先頭だけで判定を確定せず、末尾も確認します。

短辺が 32 より大きく、長辺が 32768 以上かつ短辺の 32 倍以上のときは、[非対称畳み込み](https://qandori.github.io/cp-library/components/fps_convolution_unbalanced.hpp.html) を使います。短辺の変換を再利用して長辺をブロックごとに処理し、短辺を $$m$$、長辺を $$n$$ として時間は $$O((n+m)\log m)$$、追加作業領域は $$O(m)$$ です。

次数 8 の環を葉とし、2 本の前進変換・葉の積・逆変換を部分木内で続けて処理します。条件を満たす長さでは最下段を 2 群ずつ処理し、半分が零の最上段と最後の倍率を融合します。入力の共通因子 $$R=2^{32}$$ を保持し、結果を直接 Montgomery 表現で返します。

`Subtrees`・`forward_radix8` は Taylor shift の middle product でも共有します。作業配列・root 表は RAII で管理し、assembly が読む末尾の余白を含めて初期化します。

専用 `Product` と汎用 `ntt::Convolution` のどちらでも、返す `vector<Mint>` 内に変換長と末尾 16 係数の領域を確保し、32 byte 境界から作業配列 A として使います。全係数の生存期間を開始して零で埋めてから入力をコピーし、変換後は先頭へ移動して公開サイズに縮めます。別の結果配列へのコピーを省き、返す配列の容量には変換用の余白が残ることがあります。

内部の `Product(n,m,borrowed_a)` と `ntt::Convolution(n,m,borrowed_a)` は A を借用し、B と root 表だけを所有します。`borrowed_a` は 32 byte alignment、`length()+16` 個以上の生存中の 4 byte 係数、零初期化済みの末尾を必要とします。呼び出し側が入力を正規化してコピーし、plan の利用中は領域を維持します。借用引数のない二つのコンストラクタは従来どおり A も所有し、借用のない raw 配列用の経路を維持します。

借用領域への入力コピーは `Mint` と `memcpy` を使います。変換中の SIMD アクセスは GCC の alias 対応 vector 型を使い、公開入力の `const` 領域には書き込みません。最終係数は正規化した Montgomery 表現です。

出力長を $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。
