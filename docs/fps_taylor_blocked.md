---
title: Taylor Shift の中間積
documentation_of: //components/fps_taylor_blocked.hpp
---

`fps/taylor_shift.hpp` が使う内部部品です。Linux x86_64 / GCC で定義され、呼び出し側が AVX2 対応を確認します。

`fps_detail::taylor_detail::shift(f,c)` は内部 `Mint` の係数列 `f` を $f(x+c)$ の係数列へ書き換えます。前提は $256\leq |f|\leq 2^{19}$、$c\neq0$、法 $998244353$ です。長さは変わりません。公開関数は範囲外や他の環境で既存実装を使います。

32 本の独立した積の列で階乗と重みを計算し、Taylor shift を循環積の上半分に配置します。畳み込みの変換・葉の積は `fps_convolution.hpp` と共有し、出力重みと最後の差の計算を融合します。確保は既存 FPS 作業領域の RAII を使い、SIMD が読む padding は所有領域内で初期化します。係数列に対する線形性を使い、入力の Montgomery 共通因子を出力まで保持して往復変換を省きます。

係数数を $n$ として、時間は $O(n\log n)$、追加空間は $O(n)$ です。
