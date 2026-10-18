---
title: FPS の融合畳み込み
documentation_of: //components/fps_convolution.hpp
---

Linux x86_64 / GCC の `fps::multiply` と `convolution(vector<mint>,vector<mint>)` が使う内部部品です。公開 API から利用し、呼び出し側が AVX2 対応を実行時に確認します。

`fps_detail::multiply_detail::multiply(a,b)` は、非空の Montgomery 係数列から積を返します。出力長は $2^{23}$ 以下とし、入力を変更しません。同一配列の二乗にも対応します。入力に alignment や padding は要求しません。

次数 8 の環を葉とし、2 本の前進変換・葉の積・逆変換を部分木内で続けて処理します。条件を満たす長さでは最下段を 2 群ずつ処理し、半分が零の最上段と最後の倍率を融合します。入力の共通因子 $R=2^{32}$ を保持し、結果を直接 Montgomery 表現で返します。

`Subtrees`・`forward_radix8` は Taylor shift の middle product でも共有します。作業配列・root 表は RAII で管理し、assembly が読む末尾の余白を含めて初期化します。

出力長を $n$ として、時間は $O(n\log n)$、追加空間は $O(n)$ です。
