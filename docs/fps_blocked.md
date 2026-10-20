---
title: FPS のブロック変換
documentation_of: //components/fps_blocked.hpp
---

FPS の逆数・対数・指数・冪・平方根・除算と各専用アルゴリズムが共有する内部部品です。公開 API から利用します。Linux x86_64 / GCC 用に定義され、呼び出し側が AVX2 対応を実行時に確認します。他の環境では既存の FPS 実装を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `native_log(in,input,out,n,integrate)` | 対数または対数微分を空の out へ追加 | $$O(n\log n)$$ |
| `Arena::zero_initialized()` | 確保直後の領域が零で初期化されていたか | $$O(1)$$ |

通常の利用では [FPS](https://qandori.github.io/cp-library/fps/fps.hpp.html) の `f.log(n)` を呼びます。以下の関数は `fps_detail::blocked_poly` 内の内部 API です。

`fps_detail::blocked_poly::native_log(in,input,out,n,integrate)` は、`in` の `input` 個の内部 Montgomery 係数から、対数微分の最初の `n-1` 項を求めます。`integrate=true` では積分した対数を、偽では定数項 0 の後に対数微分を格納します。結果は空の `vector<Mint>` である `out` に追加し、長さを `n` にします。`in` は変更せず、`out` と重ねません。公開 FPS 側では `256 <= n <= 2^24`、入力長 64 以上の条件で使います。

入力の範囲外は 0 とし、出力次数より後ろは読みません。係数の内部ワードは $$[0,2p)$$、定数項は非零です。入力全体の共通因子 $$R=2^{32}$$ は $$f'/f$$ で相殺されるので、入力の Montgomery 解除は行いません。積分の倍率に $$R$$ を組み込み、出力を直接 Montgomery 表現に戻します。ここで $$p=998244353$$ です。

`n >= 262144` では、初期逆数に必要な先頭部分だけを作業配列へコピーします。後続の最上段変換と微分は `const Mint` 入力を直接読み、正規化も同じ走査で行います。端数は入力の範囲内だけを読みます。小さい次数は既存のコピー経路を使います。

`blocked_poly::Transform` は root と Shoup quotient を隣接配置した、次数 8 の環を葉とする部分変換です。葉の積と最下段を融合し、最上段で倍率と必要な半分の復元を行います。正規化係数は定数表から取得し、半分が零の最上段は radix 8 まで融合します。`Arena::zero_initialized()` は確保した領域の初期状態が零かを返し、領域を再利用した後の状態は保証しません。`Arena` は既存の FPS 作業領域を使い、64 byte の間隔を空けて配列を配置します。確保失敗時は通常の整列配列へフォールバックし、それも失敗した場合は例外を送出します。公開配列に padding やアラインメントは要求しません。

入力と出力の最大係数数を $$n$$ として、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。
